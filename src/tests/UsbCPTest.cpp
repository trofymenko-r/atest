/*
 * UsbCPTest.cpp
 *
 *  Created on: Oct 25, 2019
 *      Author: ruslantrofymenko
 */

#include <unistd.h>

#include <UsbCPTest.h>
#include <ManualButton.h>

CUsbCPTest::CUsbCPTest(SEnvironment& Env):CTest(Env)
{
	Name = "usb_carplay";
}

CUsbCPTest::~CUsbCPTest()
{

}

bool CUsbCPTest::Init()
{
	if (!CTest::Init())
		return false;

	/* disconnect USB */
	Env.pUsbConnector->Set(0);

	return true;
}

bool CUsbCPTest::Run()
{
	string CycleDir;
	string Cycle;
	const int CycleCount = 1000;
	CManualButton* pButton = new CManualButton("\rconnect USB cable and press any key",
			"\rdisconnect USB cable and press any key");
	StartTime = time(nullptr);

	system("setterm -cursor off");
	Log(Common, cout, "target cycles: " + to_string(CycleCount));
	sleep(2);

	for (int ii = 0; ii < CycleCount; ii++) {
		LogOut(Common, cout, "*****************************");
		Cycle = "connect_cycle" + to_string(ii);
		Log(Common, cout, Cycle + ": start");

		CycleDir = LogDir + "/" + Cycle;
		mkdir(CycleDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

		Env.Serial.SaveOutput(CycleDir + "/serial");

		/* connect USB */
		//Env.pUsbConnector->Set(0x7);
		pButton->On();

		/* wait host connection */
		Env.Serial.ScanOutput("android_work: sent uevent USB_STATE=CONFIGURED", CSerial::EScanType::ST_EXPECTED, true);
		CSerial::EScanResult Result = Env.Serial.WaitOutput(5*1000);
		if (Result != CSerial::EScanResult::SR_OK) {
			Log(Common, cerr, "warning: host connection not detected");
			failed = true;
		} else
			Log(Common, cerr, "host connection detected");
		Env.Serial.ResetScaner();

		Env.Serial.ScanOutput("msm_vidc: info: Opening video instance:", CSerial::EScanType::ST_EXPECTED, true);
		Result = Env.Serial.WaitOutput(10*1000);
		if (Result != CSerial::EScanResult::SR_OK) {
			Log(Common, cerr, "warning: CarPlay video mode not detected");
			failed = true;
		} else
			Log(Common, cerr, "CarPlay video mode detected");
		Env.Serial.ResetScaner();

		Env.Serial.BlankLine(3);
		if (!Env.Serial.SendCmdMatch("ifconfig", "usb0      Link encap:Ethernet")) {
			Log(Common, cerr, "warning: ncm interface not detected");
		}
		Env.Serial.Send("lsusb");

		sleep(1);

		/* disconnect USB */
		//Env.pUsbConnector->Set(0);
		//pButton->Off();
		cout << "disconnect USB cable" << endl;

		/* wait host disconnection */
		Env.Serial.ScanOutput("android_work: sent uevent USB_STATE=SUSPEND", CSerial::EScanType::ST_EXPECTED, true);
		Result = Env.Serial.WaitOutput(5*1000);
		if (Result != CSerial::EScanResult::SR_OK) {
			Log(Common, cerr, "warning: host disconnection not detected");
			failed = true;
		} else
			Log(Common, cerr, "host disconnection detected");
		Env.Serial.ResetScaner();

		Log(Common, cout, Cycle + ": finish");

		sleep(1);

//		if (failed)
//			break;

		if (ii == (CycleCount - 1))
			completed = true;

	}

	Common.close();
	system("setterm -cursor on");

	return (!failed && completed);
}
