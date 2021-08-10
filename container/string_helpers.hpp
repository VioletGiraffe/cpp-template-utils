#pragma once

#include <string>

[[nodiscard]] inline constexpr bool operator==(const std::string str, const char ch) noexcept
{
	return str.size() == 1 && str.front() == ch;
}

[[nodiscard]] inline constexpr bool operator==(const char ch, const std::string str) noexcept
{
	return str == ch;
}