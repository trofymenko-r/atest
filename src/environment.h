/*
 * environment.h
 *
 *  Created on: Oct 16, 2019
 *      Author: ruslantrofymenko
 */

#ifndef ENVIRONMENT_H_
#define ENVIRONMENT_H_

#include <Serial.h>
#include <ICSEButton.h>

typedef struct {
	CSerial Serial;
	CSerial VcpuSerial;
	CICSEButton* pPowerControl;
	CICSEButton* pIgnControl;
	sys::CICSEControl* pUsbConnector;
}SEnvironment;

#endif /* ENVIRONMENT_H_ */
