#pragma once
#include <fstream>
using namespace std;
class LOG
{
public:
	LOG(const char* logFileName)
	{
		output.open(logFileName);
	}
	~LOG()
	{
		output.close();
	}
	void Log(const char* log)
	{
		output<<log<<endl;
	}
private:
	ofstream output;
};