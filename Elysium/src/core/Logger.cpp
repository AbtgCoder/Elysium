#include "Logger.h"

#include <iostream>
#include <chrono>
#include <string>
#include <time.h>
#include <Windows.h>

std::vector<LogEntry> Logger::m_Logs;

void Logger::Log(const std::string& log, const std::string& logger, LOG_TYPE type)
{
	if (!m_Logs.empty() && m_Logs.back().message == log)
	{
		m_Logs.back().count += 1;
		return;
	}

	char buff[100];
	time_t now = time(0);
	struct tm timeinfo;

	localtime_s(&timeinfo, &now);
	strftime(buff, 100, "%H:%M:%S", &timeinfo);

	LogEntry newLog = {
		type,
		buff,
		log,
		logger,
		0
	};

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	WORD color = 7;
	switch (type)
	{
	case VERBOSE:
		color = 15; // White
		break;
	case WARNING:
		color = 14; // Yellow
		break;
	case CRITICAL:
		color = 12; // Red
		break;
	case COMPILATION:
		color = 11; // Cyan
		break;
	default:
		break;
	}
	
	SetConsoleTextAttribute(hConsole, color);
	std::string msg = "[" + std::string(buff) + "] " + logger + ": " + log;
	std::cout << msg << "\n";
	SetConsoleTextAttribute(hConsole, 7);
	
	if (m_Logs.size() >= MAX_LOGS)
		m_Logs.erase(m_Logs.begin());

	m_Logs.push_back(newLog);
}
