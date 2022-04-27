#pragma once

#include <string>
#include <type_traits>

template<class CharT, class CharTraits, class Alloc>
constexpr std::basic_string<CharT, CharTraits, Alloc>& operator<<(std::basic_string<CharT, CharTraits, Alloc>& str, const char* s)
{
	return str += s;
}

template<class CharT, class CharTraits, class Alloc>
constexpr std::basic_string<CharT, CharTraits, Alloc>& operator<<(std::basic_string<CharT, CharTraits, Alloc>& str, const std::basic_string<CharT, CharTraits, Alloc>& s)
{
	return str += s;
}

template <class CharT, class CharTraits, class Alloc, typename T, std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>* = nullptr>
constexpr std::basic_string<CharT, CharTraits, Alloc>& operator<<(std::basic_string<CharT, CharTraits, Alloc>& str, const T value)
{
	return str += std::to_string(value);
}
