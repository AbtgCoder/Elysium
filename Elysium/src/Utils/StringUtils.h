#pragma once

#include <string>
#include <vector>

class StringUtils
{
public:
	static std::string RemoveWhiteSpace(const std::string& string);
	static std::string ToUpper(const std::string& string);
	static std::vector<std::string> Split(const std::string& string, char delimiter);
};