#pragma once

#include <string>
#include <string_view>
#include <type_traits>

[[nodiscard]] inline bool operator==(const std::string& str, const char ch) noexcept
{
	return str.size() == 1 && str[0] == ch;
}

[[nodiscard]] inline bool operator==(const char ch, const std::string& str) noexcept
{
	return str == ch;
}

template<class CharT, class CharTraits, class Alloc>
inline std::basic_string<CharT, CharTraits, Alloc>& operator<<(std::basic_string<CharT, CharTraits, Alloc>& str, const char* s)
{
	return str += s;
}

template<class CharT, class CharTraits, class Alloc>
inline std::basic_string<CharT, CharTraits, Alloc>& operator<<(std::basic_string<CharT, CharTraits, Alloc>& str, const std::basic_string<CharT, CharTraits, Alloc>& s)
{
	return str += s;
}

template <class CharT, class CharTraits, class Alloc, typename T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>* = nullptr>
std::basic_string<CharT, CharTraits, Alloc>& operator<<(std::basic_string<CharT, CharTraits, Alloc>& str, const T value)
{
	return str += std::to_string(value);
}

template <class CharT, class CharTraits, class Alloc, typename T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>* = nullptr>
inline std::basic_string<CharT, CharTraits, Alloc>& operator+=(std::basic_string<CharT, CharTraits, Alloc>& str, const T value)
{
	return str << value;
}

template <class CharT, class CharTraits, class Alloc, typename T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>* = nullptr>
inline std::basic_string<CharT, CharTraits, Alloc> operator+(std::basic_string<CharT, CharTraits, Alloc> str, const T value)
{
	return str << value;
}

template<class CharT, class CharTraits, class Alloc>
inline std::basic_string<CharT, CharTraits, Alloc>& operator<<(std::basic_string<CharT, CharTraits, Alloc>& str, std::string_view sv)
{
	return str += sv;
}
