/*
 * Log.cpp
 *
 *  Created on: Sep 2, 2019
 *      Author: ruslantrofymenko
 */

#include <Log.h>

CLog::CLog() {
	// TODO Auto-generated constructor stub

}

CLog::~CLog() {
	// TODO Auto-generated destructor stub
}

bool CLog::Init()
{
	return true;
}

void CLog::Close()
{

}

void LogOut(ofstream& Log, ostream& Out, const string& Msg, bool NewLine)
{
	Log << Msg;
	Out << Msg;
	if (NewLine) {
		Log << endl;
		Out << endl;
	}
}

void UpdateProgress()
{
	static const int MaxDots = 5;
	static int CurrDot = 0;

	cout << "\r";
	for (int ii = 0; ii < MaxDots; ii++) {
		if (ii < CurrDot)
			cout << ".";
		else
			cout << " ";
	}
	cout << flush;

	if (CurrDot < (MaxDots))
		CurrDot++;
	else
		CurrDot = 0;
}

