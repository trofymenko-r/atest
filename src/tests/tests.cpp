/*
 * inittests.cpp
 *
 *  Created on: Aug 23, 2019
 *      Author: ruslantrofymenko
 */

#include <StrBaseTest.h>
#include <StrConsumptTest.h>
#include <iostream>

bool InitTests(SEnvironment& Env)
{
	CTest* pTest;
	pTest = new CStrBaseTest(Env);
	CTest::TestList.push_back(pTest);

	pTest = new CStrConsumptTest(Env);
	CTest::TestList.push_back(pTest);

	return true;
}

CTest* GetTest(string Name)
{
	for (auto pTest: CTest::TestList) {
		if (pTest->getName() == Name)
			return pTest;
	}

	return nullptr;
}

void PrintTestList()
{
	for (auto pTest: CTest::TestList) {
		if (pTest == nullptr) continue;
		cout << pTest->getName() << endl;
	}
}


