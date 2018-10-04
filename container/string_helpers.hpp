#pragma once

#include <string>

inline bool operator==(const std::string str, const char ch)
{
	return str.size() == 1 && str.front() == ch;
}

inline bool operator==(const char ch, const std::string str)
{
	return str == ch;
}