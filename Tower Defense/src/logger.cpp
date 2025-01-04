#include "logger.h"

#include <iostream>

Logger::Logger() : logFile("logs.txt")
{}

Logger::~Logger()
{
	logFile.close();
}

#ifdef DEBUG
void Logger::AddLog(std::string_view newLog, bool endLine)
{
	logFile << newLog;

	if (endLine)
	{
		logFile << "\n";
		logs.emplace_back(std::string(newLog) + "\n");
	}
	else
	{
		logs.emplace_back(std::string(newLog));
	}
}

void Logger::AddLog(const std::string &newLog, bool endLine)
{
	logFile << newLog;

	if (endLine)
	{
		logFile << "\n";
		logs.emplace_back(newLog + "\n");
	}
	else
	{
		logs.emplace_back(newLog);
	}
}

void Logger::PrintQueuedLogs()
{
	for (const auto &log : logs)
	{
		std::cout << log;
	}
}
#else
void Logger::AddLog(std::string_view newLog, bool endLine)
{
	logFile << newLog;

	if (endLine)
		logFile << "\n";
}

void Logger::AddLog(const std::string &newLog, bool endLine)
{
	logFile << newLog;

	if (endLine)
		logFile << "\n";
}
#endif