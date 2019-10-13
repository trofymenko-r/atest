/*
 * CStrTest.h
 *
 *  Created on: Aug 23, 2019
 *      Author: ruslantrofymenko
 */

#ifndef TESTS_STRBASETEST_H_
#define TESTS_STRBASETEST_H_

#include <Test.h>

class CStrBaseTest: public CTest {
public:
	CStrBaseTest(CSerial& Serial);
	virtual ~CStrBaseTest();

public:
	bool Init();
	bool Run();

private:
	ofstream Console;
};

#endif /* TESTS_STRBASETEST_H_ */
