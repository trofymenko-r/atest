//============================================================================
// Name        : atest.cpp
// Author      : Ruslan Trofymenko
// Version     :
// Copyright   : 
// Description :
//============================================================================

#include <iostream>
#include <string.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <getopt.h>
#include <Serial.h>
#include <thread>
#include <vector>
#include <unistd.h>

#include <StrBaseTest.h>
#include <ICSEButton.h>
#include <ManualButton.h>
#include <App.h>
#include <ustring.h>
#include <UsbSerial.h>
//#include <ICSEControl.h>
#include <environment.h>

using namespace std;
using namespace sys;

typedef struct{
	string test;
}SSettings;

const SSettings DefaultSettings = {"str_consumpt",       // test name
                                   };
bool IsVcpuSerial(CSerial& Serial)
{
	return Serial.SendCmdMatch("", "|DM            |");
}

bool DetectEnvironment(SEnvironment& Env)
{
	cout << "init environment" << endl;

	vector<CUsbSerial::SDeviceEntry> ttyList = CUsbSerial::GetDevicesList("ftdi_sio");
	if (ttyList.empty()) {
		cerr << "serial device not detected" << endl;
		return false;
	}

	bool bDetectSerial = false;
	bool bDetectVcpuSerial = false;
	for (auto& ttyDev: ttyList) {
		CSerial* pSerial = new CSerial();
		pSerial->Init(ttyDev.Device, B115200, CBAUD|CS8|CLOCAL|CREAD, IGNPAR);
		bool bIsVcpuSerial = IsVcpuSerial(*pSerial);
		delete pSerial;
		usleep(1000);

		if (!bDetectVcpuSerial && bIsVcpuSerial) {
			if (!Env.VcpuSerial.Init(ttyDev.Device, B115200, CBAUD|CS8|CLOCAL|CREAD, IGNPAR)) {
				cerr << "error init VCPU console " << ttyDev.Device << endl;
				return false;
			}
			cout << "detect VCPU console " << ttyDev.Device << endl;
			bDetectVcpuSerial = true;
		} else if (!bDetectSerial) {
			if (!Env.Serial.Init(ttyDev.Device, B115200, CBAUD|CS8|CLOCAL|CREAD, IGNPAR)) {
				cerr << "error init main console " << ttyDev.Device << endl;
				return false;
			}
			cout << "detect main console " << ttyDev.Device << endl;
			bDetectSerial = true;
		}
	}

    if (!bDetectSerial) {
    	cerr << "main console not detected" << endl;
    	return false;
    }

    if (!bDetectVcpuSerial)
    	cerr << "VCPU console not detected" << endl;

	return true;
}

void PrintUsage(const char* cName, const SSettings& Settings)
{
   	cout << "usage: " << cName << " [options]"         << endl
         << "options: "                                << endl
         << "\t-l/--list     list of tests "           << endl
         << "\t-t/--test     specify test by name"     << endl
   		 << "\t-h/--help     print this message"       << endl;
}

int main(int argc, char** argv) {

	SSettings Settings;
	Settings = DefaultSettings;
	const char* short_options = "hlt:?";

	const struct option long_options[] = {
		{"help",    no_argument,         NULL,    'h'},
		{"list",    no_argument,         NULL,    'l'},
		{"test",    required_argument,   NULL,    't'},
	};

	int opt;
	int option_index = -1;

	CICSEControl ICSEControl;
	if (!ICSEControl.Init()) {
		cerr << "error init ICSE device" << endl;
		exit(EXIT_FAILURE);
	}

    SEnvironment Env;
    Env.pPowerControl = new CICSEButton(ICSEControl, 1);
    Env.pIgnControl = new CICSEButton(ICSEControl, 0);
    Env.pUsbConnector = &ICSEControl;

    if (!DetectEnvironment(Env)) {
		cerr << "error init environment" << endl;
		exit(EXIT_FAILURE);
	}

	InitTests(Env);

	while ((opt=getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
		switch (opt) {
			case 'h':
				PrintUsage(argv[0], DefaultSettings);
				exit(EXIT_SUCCESS);

         	case 'l':
         		PrintTestList();
         		exit(EXIT_SUCCESS);

            case 't':
				Settings.test = string(optarg);
            	break;

            case 0:

            	break;

            case '?':
            	PrintUsage(argv[0], DefaultSettings);
            	exit(EXIT_FAILURE);

         	default:
            	break;
		};
		option_index = -1;
	};

	CTest* pTest = GetTest(Settings.test);
	if (!pTest) {
		cerr << Settings.test << " undefined" << endl;
		exit(EXIT_FAILURE);
	}

	if (!pTest->Init()) {
		cerr << "error init " << Settings.test << endl;
		exit(EXIT_FAILURE);
	}

	string Result = "successful";
	if (!pTest->Run())
		Result = "failed";

	cout << "'" << pTest->getName() << "' test completed as " << Result << endl;

	return 0;
}
