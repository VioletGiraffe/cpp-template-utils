#pragma once
#include <type_traits>

template <typename T, size_t Alignment>
struct alignas(Alignment) Aligned {
	T value;

	Aligned() = default;

	template <typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
	explicit Aligned(Args&&... args) : value(std::forward<Args>(args)...) {}

	operator T&() noexcept { return value; }
	operator const T&() const noexcept { return value; }
};

#ifdef __APPLE__
template <typename T>
using CacheLinePadded = Aligned<T, 128>;
#else
template <typename T>
using CacheLinePadded = Aligned<T, 64>;
#endif
