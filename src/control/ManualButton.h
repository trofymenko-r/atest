/*
 * ManualButton.h
 *
 *  Created on: Sep 2, 2019
 *      Author: ruslantrofymenko
 */

#ifndef CONTROL_MANUALBUTTON_H_
#define CONTROL_MANUALBUTTON_H_

#include <Button.h>
#include <iostream>

class CManualButton: public CButton {
public:
	CManualButton(const std::string& OnMessage, const std::string& OffMessage):
		OnMessage(OnMessage),
		OffMessage(OffMessage) {}
	virtual ~CManualButton() {}

private:
	std::string OnMessage;
	std::string OffMessage;

public:
	bool Init(){return true;}
	void On()
	{
		system("setterm -cursor on");
		//std::cout << "\rturn on STR button and press any key" << std::endl;
		std::cout << OnMessage << std::endl;
		std::cout << ">>";
		while (std::cin.get() != '\n');
	}
	void Off()
	{
		system("setterm -cursor on");
		//std::cout << "\rturn off STR button and press any key" << std::endl;
		std::cout << OffMessage << std::endl;
		std::cout << ">>";
		while (std::cin.get() != '\n');
	}
};

#endif /* CONTROL_MANUALBUTTON_H_ */
