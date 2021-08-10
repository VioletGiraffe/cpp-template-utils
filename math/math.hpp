#pragma once
#include "../parameter_pack/parameter_pack_helpers.hpp"

#include <algorithm>
#include <limits>
#include <type_traits>

#include <stdint.h>
#include <math.h>

namespace Math {

template <typename T>
T round(T value, int numDecimalDigits)
{
	static_assert(std::is_floating_point<T>::value, "This function is only intended for floating-point values");
	const T factor = pow(T(10), T(numDecimalDigits));
	const int64_t integer = int64_t(value * factor + T(0.5));
	return T(integer) / factor;
}

template <typename T>
T floor(T value, int numDecimalDigits)
{
	static_assert(std::is_floating_point<T>::value, "This function is only intended for floating-point values");
	const T factor = pow(T(10), T(numDecimalDigits));
	const int64_t integer = int64_t(value * factor);
	return T(integer) / factor;
}

template <typename OutType, typename InType>
constexpr typename std::enable_if_t<std::is_integral<InType>::value, OutType> floor(InType value)
{
	static_assert(std::is_integral<InType>::value, "This function is only intended for integer values");
	return static_cast<OutType>(value);
}

template <typename OutType, typename InType>
typename std::enable_if_t<std::is_floating_point<InType>::value, OutType> floor(InType value)
{
	static_assert(std::is_floating_point<InType>::value, "This function is only intended for floating-point values");
	return static_cast<OutType>(::floor(value));
}

template <typename OutType, typename InType>
constexpr typename std::enable_if<std::is_integral<InType>::value, OutType>::type round(InType value)
{
	static_assert(std::is_integral<InType>::value, "This function is only intended for integer values");
	return static_cast<OutType>(value);
}

template <typename OutType, typename InType>
typename std::enable_if<std::is_floating_point<InType>::value && std::is_floating_point<OutType>::value, OutType>::type round(InType value, bool performRounding = true)
{
	static_assert(std::is_floating_point<InType>::value, "This function is only intended for floating-point values");
	return performRounding ? static_cast<OutType>(::round(value)) : static_cast<OutType>(value);
}

template <typename OutType, typename InType>
constexpr typename std::enable_if<std::is_integral<OutType>::value && std::is_floating_point<InType>::value, OutType>::type round(InType value)
{
	static_assert(std::is_floating_point<InType>::value, "This function is only intended for floating-point values");
	const OutType integer = OutType(value + InType(0.5));
	return integer;
}

template <typename OutType, typename InType>
OutType ceil(InType value)
{
	return (OutType) ::ceil(value);
}

// Integer abs
template <typename T>
constexpr typename std::enable_if<std::is_integral<T>::value, T>::type abs(T value)
{
	if (value > std::numeric_limits<T>::min())
		return value >= 0 ? value : -value;
	else
		return std::numeric_limits<T>::max();
}

// Floating-point abs
template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type abs(T value)
{
	return ::fabs(value);
}

template<typename T>
constexpr T clamp(T lowerBoundary, T value, T upperBoundary)
{
	return value > upperBoundary ? value : (value < lowerBoundary ? lowerBoundary : value);
}

template <typename T>
constexpr T signum(T value)
{
	return (value > 0) ? T(1) : ((value < 0) ? T(-1) : T(0));
}

template <typename T, typename ResultType = T>
constexpr ResultType squared(T value)
{
	return (ResultType)value * (ResultType)value;
}

template <typename T>
constexpr bool isInRange(const T value, const T lowerBound, const T upperBound)
{
	return value >= lowerBound && value <= upperBound;
}

template <typename ResultType, typename... Args>
constexpr ResultType arithmeticMean(Args&&... args)
{
	ResultType acc = ResultType(0);
	size_t n = 0;

	pack::for_value([&](auto&& value) {
		acc += value;
		++n;
	}, std::forward<Args>(args)...);

	return acc / n;
}

template <typename ResultType, typename... Args>
constexpr ResultType geometricMean(Args&&... args)
{
	ResultType acc = ResultType(1);
	size_t n = 0;

	pack::for_value([&](auto&& value) {
		acc *= value;
		++n;
	}, std::forward<Args>(args)...);

	return (ResultType)pow(acc, 1.0 / n);
}

}
