/*
 * CTest.cpp
 *
 *  Created on: Aug 23, 2019
 *      Author: ruslantrofymenko
 */

#include <time.h>
#include <Test.h>
#include <utime.h>
#include <ustring.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <Log.h>
#include <App.h>

vector<CTest*> CTest::TestList;

CTest::CTest(CSerial& Serial):Serial(Serial), completed(false), failed(false)
{
	// TODO Auto-generated constructor stub

}

CTest::~CTest()
{
	Common.close();
}

bool CTest::Init()
{
	time_t rawtime;
	struct tm* timeinfo;

	time(&rawtime);
	timeinfo = localtime (&rawtime);
	string Time = string(asctime(timeinfo));
	RemoveNewLineChar(Time);
	ReplaceStr(Time, " ", "-");

	pIgnControl = new CICSEButton();

	LogDir = sys::CApp::GetCurrentDir() + "/logs";
	mkdir(LogDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	LogDir += "/" + Name;
	mkdir(LogDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	LogDir += "/" + Time;
	mkdir(LogDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	Common.open(LogDir + "/common");
	//Common.open("common");
	if (!Common.is_open()) {
		cerr << "common not opened" << endl;
		return false;
	}

	Serial.SetCommonLog(&Common);

	pIgnControl = new CICSEButton();
	if (!pIgnControl->Init()) {
		cerr << "IGN control not initialized" << endl;
		return false;
	}
	pIgnControl->On();
	cout << "IGN on" << endl;

	cout << "wait console output" << endl;
	if (!Serial.WaitUntilWakeup(5)) {
		cerr << "device not respond" << endl;
		return false;
	}

	cout << "wait console stable" << endl;
	if (!Serial.WaitUntilSilent(5, 40)) {
		cerr << "output flood" << endl;
		return false;
	}

	cout << "check board state" << endl;
	if (!IsInSystem()) {
		cerr << "wrong mode" << endl;
		return false;
	}

	cout << "check root mode" << endl;
	if (!IsRoot()) {
		cout << "no root mode" << endl;
		Serial.Send("su");
		if (!IsRoot()) {
			cerr << "error get root" << endl;
			return false;
		}
	}
	cout << "set log level" << endl;
	if (!IgnoreLogLevel()) {
		cerr << "device not respond (loglevel)" << endl;
		return false;
	}

	LogOut(Common, cout, "start test " + Name);

	return true;
}

bool CTest::IsInSystem()
{
	return Serial.SendCmdMatch("", "APQ8096");
}

bool CTest::IsRoot()
{
	return Serial.SendCmdMatch("whoami", "root");
}

bool CTest::IgnoreLogLevel()
{
	if (Serial.SendCmdMatch("cat /sys/module/printk/parameters/ignore_loglevel", "Y"))
		return true;

	Serial.Send("echo 1 > /sys/module/printk/parameters/ignore_loglevel");
	return Serial.SendCmdMatch("cat /sys/module/printk/parameters/ignore_loglevel", "Y");
}
void CTest::Log(ofstream& Log, ostream& Out, const string& Msg, bool NewLine)
{
	string TimeStr = "\r[" + to_string(time(nullptr) - StartTime) + "]:\t";
	LogOut(Log, Out, TimeStr + Msg, NewLine);
}

