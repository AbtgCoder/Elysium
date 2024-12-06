#include "StringUtils.h"
#include "String.h"
#include <iostream>
#include <sstream>

std::string StringUtils::RemoveWhiteSpace(const std::string& string)
{
	std::string result = string;
	result.erase(remove_if(result.begin(), result.end(), isspace), result.end());
	return result;
}

std::string StringUtils::ToUpper(const std::string& string)
{
	if (string.length() == 0)
	{
		return "";
	}

	auto split = Split(string, ' ');
	std::string result;
	for (auto& word : split)
	{
		word[0] = std::toupper(word[0]);
		result += word;
	}

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
