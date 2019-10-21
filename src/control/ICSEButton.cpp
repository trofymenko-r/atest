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

using namespace sys;

CICSEButton::CICSEButton(CICSEControl& ICSEControl, int Channel)
	:ICSEControl(ICSEControl),
	 Channel(Channel)
{

}

CICSEButton::~CICSEButton()
{

}

void CICSEButton::On()
{
	ICSEControl.SetChanel(Channel, true);
}

void CICSEButton::Off()
{
	ICSEControl.SetChanel(Channel, false);
}

