/*
 * CStrTest.cpp
 *
 *  Created on: Aug 23, 2019
 *      Author: ruslantrofymenko
 */

#include <StrBaseTest.h>
#include <ManualButton.h>
#include <unistd.h>

CStrBaseTest::CStrBaseTest(SEnvironment& Env):CTest(Env)
{
	Name = "str_base";
}

CStrBaseTest::~CStrBaseTest()
{

}

bool CStrBaseTest::Init()
{
	if (!CTest::Init())
		return false;

	return true;
}

bool CStrBaseTest::Run()
{
	//CManualButton Button;
	string CycleDir;
	string Cycle;
	const int CycleCount = 300;
	const int InStrTimeOut = 20;
	//const int OutStrTimeOut = 30;
	StartTime = time(nullptr);

	system("setterm -cursor off");
	Log(Common, cout, "target cycles: " + to_string(CycleCount));
	LogOut(Common, cout, "*****************************");
	for (int ii = 0; ii < CycleCount; ii++) {
		Cycle = "cycle" + to_string(ii);
		Log(Common, cout, Cycle + ": start");

		CycleDir = LogDir + "/" + Cycle;
		mkdir(CycleDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

		Env.Serial.SaveOutput(CycleDir + "/serial");
		Env.Serial.BlankLine(3);
		Env.pIgnControl->Off();
		Log(Common, cout, "IGN off");

		Env.Serial.ResetScaner();
		Env.Serial.ScanOutput("pre-suspend", CSerial::EScanType::ST_EXPECTED);
		//Serial.ScanOutput("Suspending console1", CSerial::EScanType::ST_EXPECTED);
		Env.Serial.ScanOutput("Suspending console(s)", CSerial::EScanType::ST_EXPECTED, true);
		CSerial::EScanResult Result = Env.Serial.WaitOutput(InStrTimeOut*1000);
		if (Result != CSerial::EScanResult::SR_OK) {
			Log(Common, cerr, "error: incorrect output");
			failed = true;
		}

		if (!Env.Serial.WaitUntilDied(10, true)) {
			Log(Common, cerr, "error: STR mode not reached");
			failed = true;
			break;
		}

		Env.Serial.WriteNotice("===== STR MODE =====\n");
		Log(Common, cout, "STR mode detected");
		sleep(10);

		Env.pIgnControl->On();
		Log(Common, cout, "IGN on");

		Env.Serial.ResetScaner();
		Env.Serial.ScanOutput("usb 1-1.3: new high-speed USB device", CSerial::EScanType::ST_EXPECTED, true);
		//Serial.ScanOutput("init.post_wakeup.sh: late-resume of init exit", CSerial::EScanType::ST_EXPECTED, true);
		Result = Env.Serial.WaitOutput(InStrTimeOut*1000);
		if (Result != CSerial::EScanResult::SR_OK) {
			failed = true;
			if (!IsInSystem()) {
				Log(Common, cerr, "error: device not awake");
				sleep(10);
				break;
			}
			Log(Common, cerr, "connection on 1-1.3 port not detected");
		} else
			Log(Common, cout, "device on 1-1.3 port detected");

		Env.Serial.BlankLine(3);

		// wait for wake up output
		//Serial.WaitUntilSilent(10, OutStrTimeOut, true);
//		Serial.BlankLine(3);
//
//		if (!IsInSystem()) {
//			Log(Common, cerr, "device not awake");
//			break;
//		}
		Log(Common, cout, "IDLE mode detected");

		// wait host connection
		Env.Serial.ResetScaner();
		Env.Serial.ScanOutput("android_work: sent uevent USB_STATE=CONFIGURED", CSerial::EScanType::ST_EXPECTED, true);
		Result = Env.Serial.WaitOutput(30*1000);
		if (Result != CSerial::EScanResult::SR_OK) {
			Log(Common, cerr, "warning: host connection not detected");
			//failed = true;
		} else
			Log(Common, cerr, "host connection detected");


		Env.Serial.ResetScaner();
		Env.Serial.ScanOutput("msm_vidc: info: Opening video instance:", CSerial::EScanType::ST_EXPECTED, true);
		Result = Env.Serial.WaitOutput(10*1000);
		if (Result != CSerial::EScanResult::SR_OK) {
			Log(Common, cerr, "warning: CarPlay video mode not detected");
			//failed = true;
		} else
			Log(Common, cerr, "CarPlay video mode detected");

		Env.Serial.WaitUntilSilent(3, 10);

		if (!Env.Serial.SendCmdMatch("ifconfig", "usb0      Link encap:Ethernet")) {
			Log(Common, cerr, "warning: ncm interface not detected");
		}
		Env.Serial.Send("lsusb");

		Log(Common, cout, Cycle + ": finish");
		LogOut(Common, cout, "*****************************");

		sleep(10);

		if (failed)
			break;

		if (ii == (CycleCount - 1))
			completed = true;
	}

	Common.close();
	system("setterm -cursor on");

	return (!failed && completed);
}

