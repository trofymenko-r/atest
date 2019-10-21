/*
 * CSerial.h
 *
 *  Created on: Aug 23, 2019
 *      Author: ruslantrofymenko
 */

#ifndef SERIAL_SERIAL_H_
#define SERIAL_SERIAL_H_

#include <mutex>
#include <atomic>
#include <termios.h>
#include <Thread.h>
#include <fstream>
#include <vector>
#include <condition_variable>
#include <fstream>

using namespace std;

class CSerial {
public:
	typedef struct {
		string Key;
		bool bNotice;
	}SScanEntry;

	enum EScanType {ST_EXPECTED, ST_UNEXPECTED};
	enum EScanResult {SR_OK, SR_NO_EXPECTED, SR_IS_UNEXPECTED, SR_TIMEOUT};

public:
	CSerial();
	virtual ~CSerial();

	class CReadThread : public sys::CThread
	{
		public:
			CReadThread(CSerial* pOwn):pOwn(pOwn){}
			virtual ~CReadThread(){}

		private:
			CSerial* pOwn;

		public:
			CSerial* getOwn() const {return pOwn;}
			void Execute(){getOwn()->Run();}
	};

public:
	bool Init(const string& ttyDev, int BaudRate, int cflag, int iflag);
	bool Send(const char ch);
	bool Send(const char* data, int bytes);
	bool SendCmd(const char* data, int bytes, string& Response, int TimeOut=10);
	bool Send(const string& Cmd, bool bNewLine=true);
	bool SendCmd(const string& Cmd, string& Response, int TimeOut=200);
	bool SendCmdMatch(const string& Cmd, const string& Response, int TimeOut=200);
	void Run();
	void SaveOutput(const string& File);
	bool WaitUntilSilent(unsigned int Time, unsigned int TimeOut, bool PrintProgres = false);
	bool WaitUntilDied(unsigned int TimeOut, bool PrintProgres = false);
	bool WaitUntilWakeup(unsigned int TimeOut);
	void WriteNotice(const string& Notice);
	void BlankLine(unsigned int Count);
	bool IsAlive();
	void ScanOutput(const string& Key, EScanType Type, bool bNotice = false);
	EScanResult WaitOutput(unsigned int milliseconds);
	EScanResult CheckOutput();

	void ResetScaner()
	{
		ExpectedList.clear();
		UnExpectedList.clear();
		unexpected_count = 0;

		unique_lock<std::mutex> lk(scan_mutex);
		scan_ready = false;
		lk.unlock();
	}

	bool IsInitialized() const {return bInitialized;}

	void SetCommonLog(std::ofstream* pOut) {pCommon = pOut;}

private:
	void AddToScanList(vector<SScanEntry>& List, const SScanEntry& Entry);
	bool Scan(const string& Prev, string& Current, const string& MarkStr);

public:
	static const int buff_size = 1024*4;

private:
	int fd;
	char buffer[buff_size];
	bool bInitialized;
	mutex fd_mutex;
	CReadThread ReadThread;
	string CmdBuffer;
	atomic<bool> cmd_process;

	ofstream fs;
	string FilePath;
	atomic<bool> save_data;

	std::atomic<unsigned int> refresh_time;

	std::vector<SScanEntry> ExpectedList;    // absence leads to an error
	std::vector<SScanEntry> UnExpectedList;  // occurring leads to an error

	std::mutex scan_mutex;
	std::condition_variable scan_event;
	bool scan_ready;

	unsigned int unexpected_count;
	std::ofstream DummyCommon;
	std::ofstream *pCommon;
};

#endif /* SERIAL_SERIAL_H_ */
