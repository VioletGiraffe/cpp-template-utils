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

	constexpr operator type() const {
		return v;
	}
};

template <typename T>
using remove_cv_and_reference_t = std::remove_cv_t<std::remove_reference_t<std::remove_cv_t<T>>>;

template<class, template<class...> class>
inline constexpr bool is_specialization_of = false;

template<template<class...> class T, class... Args>
inline constexpr bool is_specialization_of<T<Args...>, T> = true;
