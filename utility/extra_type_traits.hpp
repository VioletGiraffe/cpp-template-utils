#pragma once

#include <type_traits>

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

template <typename T>
using remove_cv_and_reference_t = std::remove_cv_t<std::remove_reference_t<std::remove_cv_t<T>>>;

template<class, template<class...> class>
inline constexpr bool is_specialization_of_v = false;

template<template<class...> class T, class... Args>
inline constexpr bool is_specialization_of_v<T<Args...>, T> = true;

namespace detail {
	template <class Class, typename FieldType>
	constexpr FieldType member_type_from_ptr(FieldType Class::*) { return {}; }
}

template <auto memberPointer>
using member_type_from_ptr_t = decltype(detail::member_type_from_ptr(memberPointer));