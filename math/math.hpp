#pragma once
#include "../parameter_pack/parameter_pack_helpers.hpp"

#include <algorithm>
#include <limits>
#include <type_traits>

#include <stdint.h>
#include <math.h>

namespace Math {

template <typename T>
T round(T value, int numDecimalDigits) noexcept
{
	static_assert(std::is_floating_point<T>::value, "This function is only intended for floating-point values");
	const T factor = pow(T(10), T(numDecimalDigits));
	const int64_t integer = int64_t(value * factor + T(0.5));
	return T(integer) / factor;
}

template <typename T>
T floor(T value, int numDecimalDigits) noexcept
{
	static_assert(std::is_floating_point<T>::value, "This function is only intended for floating-point values");
	const T factor = pow(T(10), T(numDecimalDigits));
	const int64_t integer = int64_t(value * factor);
	return T(integer) / factor;
}

template <typename OutType, typename InType>
constexpr typename std::enable_if_t<std::is_integral<InType>::value, OutType> floor(InType value) noexcept
{
	static_assert(std::is_integral<InType>::value, "This function is only intended for integer values");
	return static_cast<OutType>(value);
}

template <typename OutType, typename InType>
typename std::enable_if_t<std::is_floating_point<InType>::value, OutType> floor(InType value) noexcept
{
	static_assert(std::is_floating_point<InType>::value, "This function is only intended for floating-point values");
	return static_cast<OutType>(::floor(value));
}

template <typename OutType, typename InType>
constexpr typename std::enable_if<std::is_integral<InType>::value, OutType>::type round(InType value) noexcept
{
	static_assert(std::is_integral<InType>::value, "This function is only intended for integer values");
	return static_cast<OutType>(value);
}

template <typename OutType, typename InType>
typename std::enable_if<std::is_floating_point<InType>::value && std::is_floating_point<OutType>::value, OutType>::type round(InType value, bool performRounding = true)  noexcept
{
	static_assert(std::is_floating_point<InType>::value, "This function is only intended for floating-point values");
	return performRounding ? static_cast<OutType>(::round(value)) : static_cast<OutType>(value);
}

template <typename OutType, typename InType>
constexpr typename std::enable_if<std::is_integral<OutType>::value && std::is_floating_point<InType>::value, OutType>::type round(InType value)  noexcept
{
	static_assert(std::is_floating_point<InType>::value, "This function is only intended for floating-point values");
	const OutType integer = OutType(value + InType(0.5));
	return integer;
}

template <typename OutType, typename InType>
OutType ceil(InType value) noexcept
{
	return (OutType) ::ceil(value);
}

// Integer abs
template <typename T>
constexpr typename std::enable_if<std::is_integral<T>::value, T>::type abs(T value) noexcept
{
	if (value > std::numeric_limits<T>::min())
		return value >= 0 ? value : -value;
	else
		return std::numeric_limits<T>::max();
}

// Floating-point abs
template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type abs(T value) noexcept
{
	return ::fabs(value);
}

template<typename T>
constexpr T clamp(T lowerBoundary, T value, T upperBoundary) noexcept
{
	return value > upperBoundary ? value : (value < lowerBoundary ? lowerBoundary : value);
}

template <typename T>
constexpr T signum(T value) noexcept
{
	return (value > 0) ? T(1) : ((value < 0) ? T(-1) : T(0));
}

template <typename T, typename ResultType = T>
constexpr ResultType squared(T value) noexcept
{
	return (ResultType)value * (ResultType)value;
}

template <typename T>
constexpr bool isInRange(const T value, const T lowerBound, const T upperBound) noexcept
{
	return value >= lowerBound && value <= upperBound;
}

template <typename ResultType, typename... Args>
constexpr ResultType arithmeticMean(Args&&... args) noexcept
{
	ResultType acc = ResultType(0);
	constexpr size_t n = sizeof...(Args);

	pack::for_value([&](auto&& value) {
		acc += value;
	}, std::forward<Args>(args)...);

	return acc / n;
}

template <typename ResultType, typename... Args>
[[nodiscard]] constexpr ResultType geometricMean(Args&&... args) noexcept
{
	ResultType acc = ResultType(1);
	size_t n = 0;

	pack::for_value([&](auto&& value) {
		acc *= value;
		++n;
	}, std::forward<Args>(args)...);

	return (ResultType)pow(acc, 1.0 / (double)n);
}

[[nodiscard]] constexpr size_t pow2(size_t power) noexcept
{
	size_t result = 2;
	for (size_t i = 1; i < power; ++i)
		result *= 2;

	return result;
}

[[nodiscard]] inline constexpr uint64_t reduce(uint32_t value, uint32_t range) noexcept {
	return ((uint64_t)value * (uint64_t)range) >> 32;
}

} // namespace Math
