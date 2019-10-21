/*
 * StrConsumptTest.h
 *
 *  Created on: Oct 18, 2019
 *      Author: ruslantrofymenko
 */

#ifndef TESTS_STRCONSUMPTTEST_H_
#define TESTS_STRCONSUMPTTEST_H_

#include <Test.h>

class CStrConsumptTest: public CTest {
public:
	CStrConsumptTest(SEnvironment& Env);
	virtual ~CStrConsumptTest();

public:
	bool Init();
	bool Run();
};

#endif /* TESTS_STRCONSUMPTTEST_H_ */
