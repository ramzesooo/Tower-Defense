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
		logFile << std::endl;
		logs.emplace_back(std::string(newLog) + "\n");
	}
	else
	{
		logs.emplace_back(std::string(newLog));
	}
}

void Logger::PrintQueuedLogs()
{
	for (const auto& log : logs)
	{
		std::cout << log;
	}
}

void Logger::ClearLogs()
{
	logs.clear();
}
#else
void Logger::AddLog(std::string_view newLog, bool endLine)
{
	logFile << newLog;

	if (endLine)
		logFile << std::endl;
}
#endif