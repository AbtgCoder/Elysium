#pragma once

#include <iostream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <Windows.h>


// time to string
inline std::string currentTime()
{
	auto now = std::chrono::system_clock::now();
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);
	std::tm now_tm;
	localtime_s(&now_tm, &now_time);
	std::ostringstream oss;
	oss << std::put_time(&now_tm, "[%H:%M:%S]");
	return oss.str();
}

template <typename T>
void logImpl(std::ostringstream& oss, T value) {
	oss << value;
}

// Recursive case: log the first parameter and recurse on the rest
template <typename T, typename... Args>
void logImpl(std::ostringstream& oss, T first, Args... args) {
	oss << first << " ";
	logImpl(oss, args...);
}

template<typename ...Args>
inline void logWithTime(WORD color, Args ...args)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, color);
	
	std::ostringstream oss;
	oss << currentTime() << " ";
	logImpl(oss, args...);
	oss << "\n";

	std::cout << oss.str();
	SetConsoleTextAttribute(hConsole, 7);
}

#define ESM_LOG(...)    logWithTime(15, __VA_ARGS__)
#define ESM_INFO(...)   logWithTime(10, __VA_ARGS__)
#define ESM_ERROR(...)  logWithTime(12, __VA_ARGS__)