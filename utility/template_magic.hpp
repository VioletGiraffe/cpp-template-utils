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

	constexpr type to_value() const noexcept {
		return v;
	}

#if !defined __clang__ && _MSC_VER < 1927
	constexpr operator auto() const noexcept -> type {
		return v;
	}
#else
	constexpr operator type() const noexcept {
		return v;
	}
#endif
};

template <typename...>
constexpr bool false_v = false;

template <bool condition>
using sfinae = std::enable_if_t<condition, bool>;
