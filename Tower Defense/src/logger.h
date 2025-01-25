#pragma once
#include <fstream>
#include <string>
#include <vector>

class Logger
{
public:
	Logger();
	Logger(const Logger &) = delete;
	~Logger();

	Logger &operator=(const Logger &) = delete;

	void AddLog(std::string_view newLog, bool endLine = true);
	void AddLog(const std::string &newLog, bool endLine = true);
#ifdef DEBUG
	void AddInstantLog(std::string_view newLog, bool endLine = true);
	void AddInstantLog(const std::string &newLog, bool endLine = true);
	void PrintQueuedLogs();
	inline void ClearLogs() { logs.clear(); }
#endif
private:
	std::ofstream logFile;
#ifdef DEBUG
	std::vector<std::string> logs;
#endif
};