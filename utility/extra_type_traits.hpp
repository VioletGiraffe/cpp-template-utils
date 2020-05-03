#pragma once

#include <type_traits>

// is_trivially_serializable

template <typename T>
struct is_trivially_serializable {
	using BaseT = std::remove_cv_t<T>;

	static constexpr bool value =
		!std::is_pointer_v<T> &&
		!std::is_reference_v<BaseT> &&
		std::is_same_v<BaseT, std::decay_t<BaseT>> &&
		std::is_trivially_copyable_v<BaseT> &&
		std::is_standard_layout_v<BaseT>;
};

template <typename T>
constexpr bool is_trivially_serializable_v = is_trivially_serializable<T>::value;

// remove_cv_and_reference_t

template <typename T>
using remove_cv_and_reference_t = std::remove_cv_t<std::remove_reference_t<std::remove_cv_t<T>>>;

// is_specialization_of_v

template<class, template<class...> class>
inline constexpr bool is_specialization_of_v = false;

template<template<class...> class T, class... Args>
inline constexpr bool is_specialization_of_v<T<Args...>, T> = true;

// member_type_from_ptr

namespace detail {
	template <class Class, typename FieldType>
	constexpr FieldType member_type_from_ptr(FieldType Class::*) { return {}; }
}

template <auto memberPointer>
using member_type_from_ptr_t = decltype(detail::member_type_from_ptr(memberPointer));

// is_equal_comparable

template <typename L, typename R, typename = std::void_t<>>
struct is_equal_comparable : std::false_type {};

template <typename L, typename R>
struct is_equal_comparable<L, R, std::void_t<decltype(std::declval<L>() == std::declval<R>())>> : std::true_type {};

template <typename L, typename R>
constexpr bool is_equal_comparable_v = is_equal_comparable<L, R>::value;

template <typename T>
struct type_printer {
	template <typename U = T>
	void print() {
		static_assert(fail<U>);
	}

private:
	template <typename>
	static constexpr bool fail = false;
};