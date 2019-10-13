/*
 * Log.h
 *
 *  Created on: Sep 2, 2019
 *      Author: ruslantrofymenko
 */

#ifndef LOG_LOG_H_
#define LOG_LOG_H_

#include <string>
#include <iostream>
#include <fstream>

using namespace std;

class CLog {
public:
	CLog();
	virtual ~CLog();

private:
	ofstream fs;

public:
	static bool Init();
	static void Close();

public:
	static ofstream Serial;
	static ofstream Seq;
};

void LogOut(ofstream& Log, ostream& Out, const string& Msg, bool NewLine = true);
void UpdateProgress();

#endif /* LOG_LOG_H_ */
