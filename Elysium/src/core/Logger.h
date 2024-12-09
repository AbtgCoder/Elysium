#pragma once
#include <string>
#include <vector>

enum LOG_TYPE
{
	VERBOSE,
	WARNING,
	CRITICAL,
	COMPILATION
};

struct LogEntry
{
	LOG_TYPE type;
	std::string time;
	std::string message;
	std::string logger;
	uint32_t count;
};

class Logger
{
public:
	static void Log(const std::string& log, const std::string& logger = "main", LOG_TYPE type = LOG_TYPE::VERBOSE);
	static std::vector<LogEntry> GetLogs() { return m_Logs; }
	static void ClearLogs() { m_Logs.clear(); }
	static size_t GetLogCount() { return m_Logs.size(); }
private:
	static const int MAX_LOGS = 64;
	static std::vector<LogEntry> m_Logs;
};