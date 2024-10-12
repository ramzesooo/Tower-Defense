#pragma once
//#include <iostream>
#include <fstream>
#include <string>
#include <vector>

class Logger
{
public:
	Logger();
	~Logger();

	void AddLog(std::string_view newLog, bool endLine = true);
#ifdef _DEBUG
	void PrintQueuedLogs();
	void ClearLogs();
#endif
private:
	std::ofstream logFile;
#ifdef _DEBUG
	std::vector<std::string> logs;
#endif
};