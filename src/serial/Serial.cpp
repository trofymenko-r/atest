/*
 * CSerial.cpp
 *
 *  Created on: Aug 23, 2019
 *      Author: ruslantrofymenko
 */

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <algorithm>
#include <utime.h>
#include <chrono>

#include <Serial.h>
#include <Log.h>

using namespace std;

CSerial::CSerial():fd(0),
		bInitialized(false),
		ReadThread(this),
		cmd_process(false),
		save_data(false),
		scan_ready(false),
		unexpected_count(0),
		pCommon(&DummyCommon)
{

}

CSerial::~CSerial()
{
	ReadThread.Kill();
	if (fd) close(fd);
}

bool CSerial::Init(const string& ttyDev, int BaudRate, int cflag, int iflag)
{
	struct termios tty;
	int ret;
	string DevicePath = "/dev/" + ttyDev;

	fd = open(DevicePath.c_str(), O_RDWR|O_NOCTTY|O_SYNC|O_CLOEXEC);
	if (fd < 0) {
		cerr << DevicePath << " not opened" << endl;
        return false;
	}

	ret = ioctl(fd, TIOCEXCL);
    if (ret != 0) {
        cerr << DevicePath << " error TIOCEXCL ioctl" << endl;
        return false;
    }

	tty.c_cflag = cflag;//CBAUD | CS8 | CLOCAL | CREAD;
	tty.c_iflag = iflag;//IGNPAR;
	tty.c_oflag = 0;
	tty.c_lflag = 0;//ICANON;
	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 0;
	cfsetospeed(&tty, BaudRate);
	cfsetispeed(&tty, BaudRate);

	if (tcflush(fd,TCIFLUSH) == -1)
		return false;
	if (tcflush(fd,TCOFLUSH) == -1)
		return false;
	if (tcsetattr(fd, TCSANOW, &tty) == -1)
		return false;

	ReadThread.Start();
	bInitialized = true;

	return true;
}

bool CSerial::Send(const char ch)
{
	return Send(&ch, 1);
}

bool CSerial::Send(const char* data, int bytes)
{
	int wbytes = write(fd, data, bytes);
	if (wbytes != bytes)
		return false;

	tcdrain(fd);

	return true;
}

bool CSerial::SendCmd(const char* data, int bytes, string& Response, int TimeOut)
{
	if (!Send(data, bytes))
		return false;

	CmdBuffer.clear();
	cmd_process = true;

	usleep(TimeOut*1000);
	cmd_process = false;
	Response = CmdBuffer;

	return true;
}

bool CSerial::Send(const string& Str, bool bNewLine)
{
	int bytes;
	bool bResult = false;

	if (fd == 0) return false;

	string TempStr = Str;
	if (bNewLine)
		TempStr = TempStr + "\r";

	do {
		tcflush(fd, TCIOFLUSH);
		bytes = write(fd, "\r", 3);
		if (bytes <= 0)
			break;
		tcdrain(fd);
		usleep(100*1000);

		if (!Send(TempStr.c_str(), strlen(TempStr.c_str())+1))
			break;

		bResult = true;
	} while (false);

	return bResult;
}

bool CSerial::SendCmd(const string& Cmd, string& Response, int TimeOut)
{
	if (!Send(Cmd))
		return false;

	CmdBuffer.clear();
	cmd_process = true;

	usleep(TimeOut*1000);
	cmd_process = false;
	Response = CmdBuffer;

	return true;
}

bool CSerial::SendCmdMatch(const string& Cmd, const string& Response, int TimeOut)
{
	string Resp;
	if (!SendCmd(Cmd, Resp, TimeOut))
		return false;

	if (Resp.find(Response) == string::npos) {
		cerr << "SendCmdMatch: error: expected: " << Response << " Output: " << Resp << endl;
	}
	return Resp.find(Response) != string::npos;
}

void CSerial::Run()
{
	fd_set fds;
	int ret;
	int bytes;
	string CurrBuffer;
	string PrevBuffer;

	while (true) {
		FD_ZERO(&fds);
        FD_SET(fd, &fds);
        ret = select(FD_SETSIZE, &fds, NULL, NULL, NULL);
		if (ret==1) {
			//fd_mutex.lock();
			bytes = read(fd, buffer, buff_size);
			//fd_mutex.unlock();
			if (bytes >= buff_size || bytes < 0) continue;
			buffer[bytes] = 0;
			refresh_time = time(nullptr);
			CurrBuffer = string(buffer);

			if (cmd_process)
				CmdBuffer += CurrBuffer;

			if (Scan(PrevBuffer, CurrBuffer, ">>")) {
				unique_lock<std::mutex> lk(scan_mutex);
				scan_ready = true;  // set event condition to WaitOutput
				lk.unlock();
				scan_event.notify_all();
				PrevBuffer = "";
			} else {
				PrevBuffer = CurrBuffer;
			}

			if (save_data)
				fs << CurrBuffer;
		}
	}
}

void CSerial::SaveOutput(const string& File)
{
	if (FilePath == File)
		return;

	if (save_data) {
		save_data = false;
		fs.close();
	}

	fs.open(File);
	save_data = true;
}

/*
 * Wait until the out will be silent during Time seconds
 * When TimeOut (seconds) is expired, returns false
 */
bool CSerial::WaitUntilSilent(unsigned int Time, unsigned int TimeOut, bool PrintProgres)
{
	unsigned int StartTime = time(nullptr);

	while (true) {
		if ((time(nullptr) - StartTime) >= Time &&
			(time(nullptr) - refresh_time) >= Time)
			return true;

		if ((time(nullptr) - StartTime) >= TimeOut)
			return false;

		if (PrintProgres)
			UpdateProgress();

		sleep(1);
	}
}

/*
 * Wait until the out will have died.
 * When TimeOut (seconds) is expired, function returns false;
 */
bool CSerial::WaitUntilDied(unsigned int TimeOut, bool PrintProgres)
{
	static unsigned int MinTime = 5;
	unsigned int StartTime = time(nullptr);

	if (TimeOut < MinTime)
		TimeOut = MinTime;

	if (!WaitUntilSilent(MinTime, TimeOut))
		return false;

	while (true) {
		if (!IsAlive())
			return true;

		if ((time(nullptr) - StartTime) >= TimeOut)
			return false;

		if (PrintProgres)
			UpdateProgress();

		sleep(1);
	}
}

/*
 * Wait until the output will be woke up.
 * When TimeOut (seconds) is expired, function returns false;
 */
bool CSerial::WaitUntilWakeup(unsigned int TimeOut)
{
	unsigned int StartTime = time(nullptr);

	while (true) {
		if (IsAlive())
			return true;

		if ((time(nullptr) - StartTime) >= TimeOut)
			return false;

		sleep(1);
	}
}

void CSerial::WriteNotice(const string& Notice)
{
	if (!save_data) return;

	fs << endl;
	fs << Notice << endl;
}

void CSerial::BlankLine(unsigned int Count)
{
	for (unsigned int ii = 0; ii < Count; ii++)
		Send("");
}

bool CSerial::IsAlive()
{
	string Response;
	SendCmd("", Response);

	return !Response.empty();
}

void CSerial::ScanOutput(const string& Key, EScanType Type, bool bNotice)
{
	SScanEntry Entry = {Key, bNotice};

	if (Type == EScanType::ST_EXPECTED)
		AddToScanList(ExpectedList, Entry);
	else if (Type == EScanType::ST_UNEXPECTED)
		AddToScanList(UnExpectedList, Entry);
}

void CSerial::AddToScanList(vector<SScanEntry>& List, const SScanEntry& Entry)
{
	auto ent = find_if(List.begin(), List.end(), [&Entry](SScanEntry& obj) -> bool {return obj.Key==Entry.Key;});
	if(ent!=List.end()){  // entry is exist
		(*ent).bNotice = Entry.bNotice;
		return;
	}

	List.push_back(Entry);
}

bool CSerial::Scan(const string& Prev, string& Current, const string& MarkStr)
{
	bool bResult = false;
	size_t found;
	string Text = Prev + Current;

	bool bFound;
	do {
		bFound = false;
		for (auto& Key: ExpectedList) {
			found = Text.find(Key.Key);
			if (found != string::npos) {
				if (Key.bNotice)
					bResult = true;
				ExpectedList.erase((vector<SScanEntry>::iterator)&Key);
				bFound = true;
				break; // list is changed, research in it
			}
		}
	} while (bFound);

	for (auto& Key: UnExpectedList) {
		found = Text.find(Key.Key);
		if (found != string::npos) {
			string Mark = "U" + to_string(unexpected_count);
			if (found >= Prev.length())
				found -= Prev.length();
			else {
				if ((found + Key.Key.length()) <= Prev.length())
					continue;

				found = 0;
			}

			if (Key.bNotice)
				bResult = true;

			Current.insert(found, " " + MarkStr + Mark + MarkStr);
			LogOut(*pCommon, cout, "detected unexpected output(" + Mark + "): " + Key.Key);
			unexpected_count++;
		}
	}

	return bResult;
}

CSerial::EScanResult CSerial::WaitOutput(unsigned int milliseconds)
{
	unique_lock<std::mutex> lk(scan_mutex);

	auto now = chrono::system_clock::now();
	if (scan_event.wait_until(lk, now + chrono::milliseconds(milliseconds), [&]{return scan_ready;})) {
		// on event
	} else {
		// timeout
		return EScanResult::SR_TIMEOUT;//(ExpectedList.size() == 0);
	}

	return CheckOutput();

}

CSerial::EScanResult CSerial::CheckOutput()
{
	for (auto& Key: ExpectedList) {
		LogOut(*pCommon, cerr, "absence of expected output: " + Key.Key);
	}

	if (unexpected_count != 0)
		return EScanResult::SR_IS_UNEXPECTED;

	if (ExpectedList.size() != 0)
		return EScanResult::SR_NO_EXPECTED;

	return EScanResult::SR_OK;
}


