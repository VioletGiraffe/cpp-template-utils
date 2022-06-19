#pragma once

#include <string>
#include <type_traits>

#if !defined __GNUC__ || __GNUC__ >= 12
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

template<class CharT, class CharTraits, class Alloc>
GCC_CONSTEXPR_WORKAROUND std::basic_string<CharT, CharTraits, Alloc>& operator<<(std::basic_string<CharT, CharTraits, Alloc>& str, const char* s)
{
	return str += s;
}

template<class CharT, class CharTraits, class Alloc>
GCC_CONSTEXPR_WORKAROUND std::basic_string<CharT, CharTraits, Alloc>& operator<<(std::basic_string<CharT, CharTraits, Alloc>& str, const std::basic_string<CharT, CharTraits, Alloc>& s)
{
	return str += s;
}

template <class CharT, class CharTraits, class Alloc, typename T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>* = nullptr>
GCC_CONSTEXPR_WORKAROUND std::basic_string<CharT, CharTraits, Alloc>& operator<<(std::basic_string<CharT, CharTraits, Alloc>& str, const T value)
{
	return str += std::to_string(value);
}
