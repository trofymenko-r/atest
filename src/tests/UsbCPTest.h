/*
 * UsbCPTest.h
 *
 *  Created on: Oct 25, 2019
 *      Author: ruslantrofymenko
 */

#ifndef TESTS_USBCPTEST_H_
#define TESTS_USBCPTEST_H_

#include <Test.h>

class CUsbCPTest: public CTest {
public:
	CUsbCPTest(SEnvironment& Env);
	virtual ~CUsbCPTest();

public:
	bool Init();
	bool Run();
};

#endif /* TESTS_USBCPTEST_H_ */
