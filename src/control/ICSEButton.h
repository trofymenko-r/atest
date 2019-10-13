/*
 * ICSEButton.h
 *
 *  Created on: Sep 2, 2019
 *      Author: ruslantrofymenko
 */

#ifndef CONTROL_ICSEBUTTON_H_
#define CONTROL_ICSEBUTTON_H_

#include <Button.h>
#include <Serial.h>

class CICSEButton: public CButton {
public:
	CICSEButton();
	virtual ~CICSEButton();

private:
	CSerial* pSerial;

public:
	bool Init();
	void On();
	void Off();
};

#endif /* CONTROL_ICSEBUTTON_H_ */
