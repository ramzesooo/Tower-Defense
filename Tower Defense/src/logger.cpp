#include "logger.h"

void Logger::AddLog(std::string_view newLog, bool endLine)
{
	if (endLine)
	{
		logs.push_back(std::string(newLog) + "\n");
	}
	else
	{
		logs.push_back(std::string(newLog));
	}
}

void Logger::PrintQueuedLogs()
{
	if (logs.size() <= 0)
	{
		return;
	}

	for (const auto& log : logs)
	{
		std::cout << log;
	}
}

void Logger::ClearLogs()
{
	logs.clear();
}