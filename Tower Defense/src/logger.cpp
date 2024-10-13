#include "logger.h"

#include <iostream>

Logger::Logger() : logFile("logs.txt")
{}

Logger::~Logger()
{
	logFile.close();
}

void Logger::AddLog(std::string_view newLog, bool endLine)
{
	logFile << newLog;

	if (endLine)
	{
		logFile << std::endl;
#ifdef DEBUG
		logs.emplace_back(std::string(newLog) + "\n");
#endif
	}
#ifdef DEBUG
	else
	{
		logs.emplace_back(std::string(newLog));
	}
#endif
}

#ifdef DEBUG
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
#endif