/*
 * CTest.h
 *
 *  Created on: Aug 23, 2019
 *      Author: ruslantrofymenko
 */

#ifndef TESTS_TEST_H_
#define TESTS_TEST_H_

#include <string>
#include <vector>
#include <Log.h>
#include <Serial.h>
#include <fstream>

#include <environment.h>
#include <ICSEButton.h>

using namespace std;

class CTest {
public:
	CTest(SEnvironment& Env);
	virtual ~CTest();

public:
	string getName() {return Name;}
	virtual bool Run()=0;

public:
	static vector<CTest*> TestList;

protected:
	string Name;
	string LogDir;
	SEnvironment& Env;
	ofstream Common;
	bool completed;
	bool failed;
	unsigned int StartTime;

protected:
	void Log(ofstream& Log, ostream& Out, const string& Msg, bool NewLine = true);

public:
	virtual bool Init();
	bool IsInSystem();
	bool IsRoot();
	bool IgnoreLogLevel();
};

bool InitTests(SEnvironment& Env);
CTest* GetTest(string Name);
void PrintTestList();

#endif /* TESTS_TEST_H_ */
