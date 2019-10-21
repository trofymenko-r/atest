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

CTest::CTest(SEnvironment& Env):Env(Env), completed(false), failed(false), StartTime(0)
{

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

	LogDir = sys::CApp::GetCurrentDir() + "/logs";
	mkdir(LogDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	LogDir += "/" + Name;
	mkdir(LogDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	LogDir += "/" + Time;
	mkdir(LogDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	Common.open(LogDir + "/common");
	if (!Common.is_open()) {
		cerr << "common not opened" << endl;
		return false;
	}

	Env.Serial.SetCommonLog(&Common);
	Env.VcpuSerial.SetCommonLog(&Common);

	Env.pPowerControl->On();
	cout << "power on" << endl;
	Env.pIgnControl->On();
	cout << "IGN on" << endl;

	cout << "wait console output" << endl;
	if (!Env.Serial.WaitUntilWakeup(5)) {
		cerr << "device not respond" << endl;
		return false;
	}

	cout << "wait console stable" << endl;
	if (!Env.Serial.WaitUntilSilent(5, 40)) {
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
		Env.Serial.Send("su");
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
	return Env.Serial.SendCmdMatch("", "APQ8096");
}

bool CTest::IsRoot()
{
	return Env.Serial.SendCmdMatch("whoami", "root");
}

bool CTest::IgnoreLogLevel()
{
	if (Env.Serial.SendCmdMatch("cat /sys/module/printk/parameters/ignore_loglevel", "Y"))
		return true;

	Env.Serial.Send("echo 1 > /sys/module/printk/parameters/ignore_loglevel");
	return Env.Serial.SendCmdMatch("cat /sys/module/printk/parameters/ignore_loglevel", "Y");
}

void CTest::Log(ofstream& Log, ostream& Out, const string& Msg, bool NewLine)
{
	string TimeStr = "\r[" + to_string(time(nullptr) - StartTime) + "]:\t";
	LogOut(Log, Out, TimeStr + Msg, NewLine);
}

