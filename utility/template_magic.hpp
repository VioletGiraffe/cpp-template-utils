#pragma once

#include <type_traits>

#define FAIL_COMPILATION static_assert(![]{})
#define FAIL_COMPILATION_WITH_MSG(msg) static_assert(![]{}, msg)

template <bool condition>
using sfinae = std::enable_if_t<condition, bool>;

template <typename T>
struct type_printer {
	template <typename U = T>
	void print() {
		FAIL_COMPILATION;
	}
};

template <class... Ts>
struct overload final : Ts...
{
	using Ts::operator()...;
};
