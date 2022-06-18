#pragma once

#include <type_traits>

template <typename T>
struct type_wrapper {
	using type = T;
};

template <auto v>
struct value_as_type {
	using type = decltype(v);

#if !defined __clang__ && defined(_MSC_VER) && _MSC_VER < 1927
	constexpr operator auto() const noexcept -> type {
		return v;
	}
#else
	consteval operator type() const noexcept {
		return v;
	}
#endif
};

#define FAIL_COMPILATION static_assert(![]{})
#define FAIL_WITH_MSG(msg) static_assert(![]{}, msg)

template <bool condition>
using sfinae = std::enable_if_t<condition, bool>;

template <typename T>
struct type_printer {
	template <typename U = T>
	void print() {
		FAIL_COMPILATION;
	}
};