/*
 * Button.cpp
 *
 *  Created on: Sep 2, 2019
 *      Author: ruslantrofymenko
 */

#include <unistd.h>
#include <Button.h>

CButton::CButton()
{
	// TODO Auto-generated constructor stub
}

CButton::~CButton()
{
	// TODO Auto-generated destructor stub
}

void CButton::Trigger(int Delay)
{
	On();
	usleep(Delay*1000);
	Off();
}

