#include "StringUtils.h"

#include "String.h"
#include <iostream>
#include <regex>	
#include <sstream>

std::string StringUtils::RemoveWhiteSpace(const std::string& string)
{
	std::string result = string;
	result.erase(remove_if(result.begin(), result.end(), isspace), result.end());
	return result;
}

std::string StringUtils::ToUpper(const std::string& string)
{
	std::string result = string;
	std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::toupper(c);  });
	return result;
}

std::vector<std::string> StringUtils::Split(const std::string& string, char delimiter)
{
	std::vector<std::string> result;
	std::stringstream ss(string);
	std::string item;

	while (getline(ss, item, delimiter))
	{
		result.push_back(item);
	}

	return result;
}
