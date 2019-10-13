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
#include <unistd.h>

#include <StrBaseTest.h>
#include <ICSEButton.h>
#include <ManualButton.h>
#include <App.h>
#include <ustring.h>

using namespace std;

typedef struct{
	string test;
}SSettings;

const SSettings DefaultSettings = {"str_base",       // test name
                                   };

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

//	FILE* fred = freopen("output", "w", stdout);
//	if (!fred) {
//		cerr << "error open output" << endl;
//		exit(EXIT_FAILURE);
//	}


//	CSerial* pSerial = new CSerial();
//    if (pSerial == nullptr) {
//    	cerr << "error create serial interface" << endl;
//    	return false;
//    }
//
//    if (!pSerial->Init("ttyUSB7", B115200, CBAUD|CS8|CLOCAL|CREAD, IGNPAR)) {
//    	cerr << "error init serial interface" << endl;
//    	return false;
//    }
//
//    for (int ii=0; ii<65536; ii++) {
//    	char com[2] = {(char)(ii>>8), (char)ii};
//    	string Resp;
//    	pSerial->SendCmd(com, 2, Resp, 30);
//    }
//
//    delete pSerial;


	string ttyStr = sys::CApp::Exec("find /sys/bus/usb-serial/drivers/ftdi_sio/ -name ttyUSB*");
    vector<string> ResultStrings = Split(ttyStr, '\n');

    if (ResultStrings.size() == 0) {
    	cerr << "usb-serial device not detected" << endl;
    	return false;
    }

    if (ResultStrings.size() != 1) {
    	cerr << "clarify usb-serial device:" << endl;
    	for (auto& str: ResultStrings)
    		cerr << str << endl;
    	return false;
    }

    string ttyDev = ttyStr.substr(ttyStr.find("ttyUSB"));
    RemoveNewLineChar(ttyDev);

    cout << "detected main serial device: " << ttyDev << endl;

	CSerial Serial;
	if (!Serial.Init(ttyDev, B115200, CBAUD|CS8|CLOCAL|CREAD, IGNPAR)) {
		cerr << "serial console init error" << endl;
		exit(EXIT_FAILURE);
	}

	InitTests(Serial);

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

//	CICSEButton Button;
//	Button.Init();

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



//	CButton *pStrButton = new CManualButton();//CICSEButton();
//	pStrButton->Init();
//
//	pStrButton->On();
//	pStrButton->Off();

//	string Response;
//	while (true) {
//
//		if (!Serial.SendCmd("ls -la /vendor", Response))
//			cerr << "error" << endl;
//		else
//			cout << Response << endl;
//		sleep(5);
//	}

	return 0;
}
