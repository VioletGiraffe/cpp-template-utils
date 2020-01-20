#pragma once

#include <type_traits>

namespace detail {
template <class Class, typename FieldType>
constexpr FieldType member_type_from_ptr(FieldType Class::*) {return {};}
}

template <auto memberPointer>
using member_type_from_ptr_t = decltype(detail::member_type_from_ptr(memberPointer));

template <typename T>
using remove_cv_and_reference_t = std::remove_cv_t<std::remove_reference_t<std::remove_cv_t<T>>>;

template<class, template<class...> class>
inline constexpr bool is_specialization_of_v = false;

template<template<class...> class T, class... Args>
inline constexpr bool is_specialization_of_v<T<Args...>, T> = true;
