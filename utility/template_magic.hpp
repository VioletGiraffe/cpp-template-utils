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
};

template <typename T>
using remove_cv_and_reference_t = std::remove_cv_t<std::remove_reference_t<std::remove_cv_t<T>>>;
