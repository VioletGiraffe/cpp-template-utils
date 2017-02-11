#pragma once

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
typename std::enable_if<std::is_integral<InType>::value, OutType>::type round(InType value)
{
	return static_cast<OutType>(value);
}

template <typename OutType, typename InType>
typename std::enable_if<std::is_floating_point<InType>::value && std::is_floating_point<OutType>::value, OutType>::type round(InType value, bool performRounding = true)
{
	return performRounding ? static_cast<OutType>(::round(value)) : static_cast<OutType>(value);
}

template <typename OutType, typename InType>
typename std::enable_if<std::is_integral<OutType>::value && std::is_floating_point<InType>::value, OutType>::type round(InType value)
{
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
typename std::enable_if<std::is_integral<T>::value, T>::type abs(T value)
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
T clamp(T lowerBoundary, T value, T upperBoundary)
{
	return std::min(upperBoundary, std::max(lowerBoundary, value));
}

template <typename T>
T signum(T value)
{
	return (value > 0) ? T(1) : ((value < 0) ? T(-1) : T(0));
}

template <typename T, typename ResultType = T>
ResultType squared(T value)
{
	return (ResultType)value * (ResultType)value;
}

template <typename T>
bool isInRange(const T value, const T lowerBound, const T upperBound)
{
	return value >= lowerBound && value <= upperBound;
}

namespace detail
{
	template<typename... Args> void iterate_template_pack(Args&&...) {}
}

template <typename ResultType, typename... Args>
ResultType arithmeticMean(Args&&... args)
{
	ResultType acc = ResultType(0);
	size_t n = 0;

	detail::iterate_template_pack((([&](ResultType arg) {
		acc += arg;
		++n;
	})(args), void(), 0)...);

	return acc / n;
}

template <typename ResultType, typename... Args>
ResultType geometricMean(Args&&... args)
{
	ResultType acc = ResultType(1);
	size_t n = 0;

	detail::iterate_template_pack((([&](ResultType arg) {
		acc *= arg;
		++n;
	})(args), void(), 0)...);

	return (ResultType)pow(acc, 1.0 / n);
}

}
