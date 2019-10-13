/*
 * ICSEButton.cpp
 *
 *  Created on: Sep 2, 2019
 *      Author: ruslantrofymenko
 */

#include <ICSEButton.h>
#include <App.h>
#include <ustring.h>
#include <vector>
#include <iostream>
#include <unistd.h>

CICSEButton::CICSEButton() {
	// TODO Auto-generated constructor stub

}

CICSEButton::~CICSEButton() {
	if (pSerial != nullptr) delete pSerial;
}

bool CICSEButton::Init()
{
	cout << "init ICSE" << endl;

	string Result = sys::CApp::Exec("find /sys/bus/usb-serial/drivers/pl2303/ -name ttyUSB*");
	//string Result = sys::CApp::Exec("find /sys/bus/usb-serial/devices/ -name ttyUSB*");
    vector<string> ResultStrings = Split(Result, '\n');

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

    string ttyDev = Result.substr(Result.find("ttyUSB"));
    RemoveNewLineChar(ttyDev);

    cout << "detected ICSE serial device: " << ttyDev << endl;

    pSerial = new CSerial();
    if (pSerial == nullptr) {
    	cerr << "error create serial interface" << endl;
    	return false;
    }

    if (!pSerial->Init(ttyDev, B9600, CBAUD|CS8|CLOCAL|CREAD, IGNPAR)) {
    	cerr << "error init serial interface" << endl;
    	return false;
    }

    char wdata[1] = {0x50};
    string Response;
    bool bResponse = false;
    for (int ii = 0; ii < 6; ii++) {
		if (pSerial->SendCmd(wdata, 1, Response) && Response.length() != 0) {
			bResponse = true;
			break;
		}
		usleep(5*1000);
    }

    if (!bResponse) {
    	cerr << "ICSE not response" << endl;
    	return false;
    }

    cout << "ICSE code: " << +static_cast<unsigned char>(*Response.c_str()) << endl;

    switch(static_cast<unsigned char>(*Response.c_str())) {
    case 0xAB:
    	cout << "detect ICSE012A" << endl;
    	break;
    case 0xAD:
    	cout << "detect ICSE013A" << endl;
    	break;
    case 0xAC:
    	cout << "detect ICSE014A" << endl;
    	break;
    default:
		cerr << "undefined ICSE device" << endl;
		return false;
    }
    pSerial->Send(0x51);
    usleep(1000);

//    char Value = 0xFE;
//    while (true) {
//    	pSerial->Send(Value);
//    	pSerial->Send(0xFF);
//    }
//    pSerial->Send(0xFE);
//    pSerial->Send(0xFF);

	return true;
}

#define RELAY_NUMBER 0
void CICSEButton::On()
{
	if (pSerial == nullptr)
		return;
	pSerial->Send(~((unsigned char)(1 << RELAY_NUMBER)));
}

void CICSEButton::Off()
{
	if (pSerial == nullptr)
		return;
	pSerial->Send(0xFF);
}

