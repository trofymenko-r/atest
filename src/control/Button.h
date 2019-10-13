/*
 * Button.h
 *
 *  Created on: Sep 2, 2019
 *      Author: ruslantrofymenko
 */

#ifndef CONTROL_BUTTON_H_
#define CONTROL_BUTTON_H_

class CButton {
public:
	CButton();
	virtual ~CButton();

public:
	virtual bool Init()=0;
	virtual void On()=0;
	virtual void Off()=0;
	void Trigger(int Delay=200);
};

#endif /* CONTROL_BUTTON_H_ */
