#pragma once

#include <string>

#if !defined __GNUC__ || __GNUC__ >= 11
#define GCC_CONSTEXPR_WORKAROUND constexpr
#else
#define GCC_CONSTEXPR_WORKAROUND
#endif

[[nodiscard]] inline GCC_CONSTEXPR_WORKAROUND bool operator==(const std::string& str, const char ch) noexcept
{
	return str.size() == 1 && str[0] == ch;
}

[[nodiscard]] inline GCC_CONSTEXPR_WORKAROUND bool operator==(const char ch, const std::string& str) noexcept
{
	return str == ch;
}
