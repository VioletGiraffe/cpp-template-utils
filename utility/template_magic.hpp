#pragma once

#include <type_traits>

template <typename T>
struct type_wrapper {
	using type = T;
};

template <auto v>
struct value_as_type {
	static constexpr auto value = v;
	using type = decltype(value);

#ifdef _MSC_VER
	constexpr operator auto() -> type const {
		return v;
	}
#else
	constexpr operator type() const {
		return v;
	}
#endif
};

template <typename>
constexpr bool false_v = false;

template <bool condition>
using sfinae = std::enable_if_t<condition, bool>;