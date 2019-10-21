/*
 * ICSEButton.h
 *
 *  Created on: Sep 2, 2019
 *      Author: ruslantrofymenko
 */

#ifndef CONTROL_ICSEBUTTON_H_
#define CONTROL_ICSEBUTTON_H_

#include <Button.h>
#include <ICSEControl.h>

class CICSEButton: public CButton {
public:
	CICSEButton(sys::CICSEControl& ICSEControl, int Channel);
	virtual ~CICSEButton();

private:
	sys::CICSEControl& ICSEControl;
	int Channel;

private:
	bool Init(){return true;}

public:
	void On();
	void Off();
};

#endif /* CONTROL_ICSEBUTTON_H_ */
