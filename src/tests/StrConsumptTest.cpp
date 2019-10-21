/*
 * StrConsumptTest.cpp
 *
 *  Created on: Oct 18, 2019
 *      Author: ruslantrofymenko
 */

#include <sys/stat.h>
#include <unistd.h>

#include <StrConsumptTest.h>

CStrConsumptTest::CStrConsumptTest(SEnvironment& Env):CTest(Env)
{
	Name = "str_consumpt";
}

CStrConsumptTest::~CStrConsumptTest()
{

}

bool CStrConsumptTest::Init()
{
	if (!CTest::Init())
		return false;

	return true;
}

bool CStrConsumptTest::Run()
{
	string CycleDir;
	string Cycle;
	const int CycleCount = 300;
	StartTime = time(nullptr);

	system("setterm -cursor off");
	Log(Common, cout, "target cycles: " + to_string(CycleCount));

	for (int ii = 0; ii < CycleCount; ii++) {
		LogOut(Common, cout, "*****************************");
		Cycle = "power_cycle" + to_string(ii);
		Log(Common, cout, Cycle + ": start");

		CycleDir = LogDir + "/" + Cycle;
		mkdir(CycleDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

		Env.Serial.SaveOutput(CycleDir + "/serial");
		Env.VcpuSerial.SaveOutput(CycleDir + "/vcpu_serial");

		Env.pPowerControl->Off();
		Log(Common, cout, "power off");
		sleep(1);
		Env.pPowerControl->On();
		Log(Common, cout, "power on");

		Env.VcpuSerial.ScanOutput("|0A 00 01", CSerial::EScanType::ST_UNEXPECTED);

		Env.Serial.ScanOutput("booting linux @ 0x80080000,", CSerial::EScanType::ST_EXPECTED, true);
		CSerial::EScanResult Result = Env.Serial.WaitOutput(10*1000);
		if (Result == CSerial::EScanResult::SR_OK) {
			Log(Common, cout, "detect booting kernel");

		} else {
			Log(Common, cerr, "warning: booting kernel not detected");
			failed = true;
		}
		Env.Serial.ResetScaner();

		Env.Serial.ScanOutput("Initializing Scheduler", CSerial::EScanType::ST_EXPECTED, true);
		Result = Env.Serial.WaitOutput(20*1000);
		if (Result == CSerial::EScanResult::SR_OK) {
			Log(Common, cout, "detect system");

		} else {
			Log(Common, cerr, "warning: system not detected");
			failed = true;
		}
		Env.Serial.ResetScaner();

		Env.Serial.Send("su");
		usleep(200*1000);
		if (!IgnoreLogLevel())
			Log(Common, cerr, "warning: ignore loglevel");

		Env.Serial.ScanOutput("BSS Key setup with vdev_mac", CSerial::EScanType::ST_EXPECTED, true);
		Result = Env.Serial.WaitOutput(40*1000);
		if (Result == CSerial::EScanResult::SR_OK) {
			Log(Common, cout, "boot successful");

		} else {
			Log(Common, cerr, "warning: boot unsuccessful");
			failed = true;
		}
		Env.Serial.ResetScaner();

		sleep(5);
		Env.Serial.BlankLine(3);

		if (failed)
			break;

		if (ii == (CycleCount - 1))
			completed = true;

		Result = Env.VcpuSerial.CheckOutput();
		Env.VcpuSerial.ResetScaner();
		if (Result == CSerial::EScanResult::SR_OK) {
			Log(Common, cout, "VCPU self-reboot not detected");
			sleep(1);
			continue;
		}

		//=======================================================-

		Log(Common, cout, "detected VCPU self-reboot");
		Log(Common, cout, "go to suspend");
		sleep(1);
		Env.pIgnControl->Off();
		Log(Common, cout, "IGN off");


		Env.Serial.ScanOutput("pre-suspend", CSerial::EScanType::ST_EXPECTED, true);
		Result = Env.Serial.WaitOutput(15*1000);
		if (Result == CSerial::EScanResult::SR_OK) {
			Log(Common, cout, "detected start of suspend");

		} else {
			Log(Common, cerr, "warning: start of suspend not detected");
			failed = true;
		}
		Env.Serial.ResetScaner();

		Env.Serial.ScanOutput("Suspending console(s)", CSerial::EScanType::ST_EXPECTED, true);
		Result = Env.Serial.WaitOutput(20*1000);
		if (Result == CSerial::EScanResult::SR_OK) {
			Log(Common, cout, "detected suspend");

		} else {
			Log(Common, cerr, "warning: suspend not detected");
			failed = true;
		}
		Env.Serial.ResetScaner();

		// suspend reboot

		Env.Serial.ScanOutput("] platform_init()", CSerial::EScanType::ST_EXPECTED, true);
		Result = Env.Serial.WaitOutput(20*1000);
		if (Result == CSerial::EScanResult::SR_OK) {
			Log(Common, cout, "detected reboot");

		} else {
			Log(Common, cerr, "warning: reboot not detected");
			failed = true;
		}
		Env.Serial.ResetScaner();

		Env.Serial.ScanOutput("Initializing Scheduler", CSerial::EScanType::ST_EXPECTED, true);
		Result = Env.Serial.WaitOutput(40*1000);
		if (Result == CSerial::EScanResult::SR_OK) {
			Log(Common, cout, "detect system");

		} else {
			Log(Common, cerr, "warning: system not detected");
			failed = true;
		}
		Env.Serial.ResetScaner();

		Env.Serial.Send("su");
		usleep(200*1000);
		if (!IgnoreLogLevel())
			Log(Common, cerr, "warning: ignore loglevel");

		Env.Serial.Send("dmesg -C");

		Env.Serial.ScanOutput("Suspending console(s)", CSerial::EScanType::ST_EXPECTED, true);
		Result = Env.Serial.WaitOutput(40*1000);
		if (Result == CSerial::EScanResult::SR_OK) {
			Log(Common, cout, "detected suspend");

		} else {
			Log(Common, cerr, "warning: suspend not detected");
			failed = true;
		}
		Env.Serial.ResetScaner();

		sleep(100);

		break;
	}

	Common.close();
	system("setterm -cursor on");

	return (!failed && completed);
}
