#pragma once

#include <type_traits>

// is_trivially_serializable

namespace detail {
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
}

template <typename T>
inline constexpr bool is_trivially_serializable_v = detail::is_trivially_serializable<T>::value;

// remove_cv_and_reference_t

template <typename T>
using remove_cv_and_reference_t = std::remove_cv_t<std::remove_reference_t<std::remove_cv_t<T>>>;

// is_specialization_of_v<Concrete, Template> is 'true' if Concrete is a specialization of Template

template<class, template<class...> class>
inline constexpr bool is_specialization_of_v = false;

template<template<class...> class T, class... Args>
inline constexpr bool is_specialization_of_v<T<Args...>, T> = true;

// member_type_from_ptr - given a pointer to the class data member, retrieves the type of this member

namespace detail {
	template <class Class, typename FieldType>
	FieldType member_type_from_ptr(FieldType Class::*); // No definition needed
}

template <auto memberPointer>
using member_type_from_ptr_t = decltype(detail::member_type_from_ptr(memberPointer));

// is_equal_comparable - is a type equal comparable to another type

template <typename L, typename R, typename = void>
struct is_equal_comparable : std::false_type {};

template <typename L, typename R>
struct is_equal_comparable<L, R, std::enable_if_t<std::is_same_v<bool, decltype(std::declval<L>() == std::declval<R>())>>> : std::true_type {};

template <typename L, typename R>
inline constexpr bool is_equal_comparable_v = is_equal_comparable<L, R>::value;

// is_sortable_container - true if the type provides random access iterators

template <typename C>
inline constexpr bool is_sortable_container_v = std::is_same_v<std::random_access_iterator_tag, typename std::iterator_traits<typename C::iterator>::iterator_category>;
