#pragma once
#include <iostream>
#include <vector>

class Logger
{
public:
	void AddLog(std::string_view newLog, bool endLine = true);
	//void NewLine() { logs.push_back("\n"); }
	void NewLine() { logs.back() += "\n"; } // probably it's not going to be used
	void PrintQueuedLogs();
	void ClearLogs();
private:
	std::vector<std::string> logs;
};