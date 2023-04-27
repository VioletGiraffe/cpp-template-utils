#pragma once

/*
Capture into lambda by moving:

auto object = get_obj();
auto lambda = [mv(object)] {};

*/
#define mv(name) name=std::move(name)

#define NON_COPYABLE(T) \
	T(const T&) = delete; \
	T& operator=(const T&) = delete; \
	constexpr T(T&&) noexcept = default; \
	constexpr T& operator=(T&&) noexcept = default

#define NON_MOVABLE(T) \
	T(const T&) = delete; \
	T& operator=(const T&) = delete; \
	T(T&&) = delete; \
	T& operator=(T&&) = delete
