#pragma once
#include "../parameter_pack/parameter_pack_helpers.hpp"
#include "../utility/extra_type_traits.hpp"

#include <algorithm>
#include <iterator>
#include <limits>
#include <type_traits>
#include <utility>

#include <assert.h>
#include <stdint.h>
#include <math.h>

#ifdef _MSC_VER
#include <intrin.h>
#endif

namespace Math {

namespace detail {

// Exact for every power in the table, unlike pow() (also cheap, unlike pow)
template <typename T>
[[nodiscard]] constexpr T powerOf10(int power) noexcept
{
	// 10^22 is the largest power of ten a double holds exactly: 10^n = 2^n * 5^n, and 5^23 exceeds the mantissa
	constexpr double powers[] = {
		1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11,
		1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22
	};

	constexpr int largestExactPower = int(std::size(powers)) - 1;

	if (power >= 0 && power <= largestExactPower)
		return T(powers[power]);
	else if (power < 0 && power >= -largestExactPower)
		return T(1.0 / powers[-power]);

	return T(::pow(10.0, double(power)));
}

} // namespace detail

template <typename T>
T round(T value, int numDecimalDigits) noexcept
{
	static_assert(std::is_floating_point<T>::value, "This function is only intended for floating-point values");
	const T factor = detail::powerOf10<T>(numDecimalDigits);
	return ::round(value * factor) / factor;
}

template <typename T>
T floor(T value, int numDecimalDigits) noexcept
{
	static_assert(std::is_floating_point<T>::value, "This function is only intended for floating-point values");
	const T factor = detail::powerOf10<T>(numDecimalDigits);
	return ::floor(value * factor) / factor;
}

template <typename OutType, typename InType>
constexpr typename std::enable_if_t<std::is_integral<InType>::value, OutType> floor(InType value) noexcept
{
	return static_cast<OutType>(value);
}

template <typename OutType, typename InType>
typename std::enable_if_t<std::is_floating_point<InType>::value, OutType> floor(InType value) noexcept
{
	return static_cast<OutType>(::floor(value));
}

template <typename OutType, typename InType>
constexpr typename std::enable_if<std::is_integral<InType>::value, OutType>::type round(InType value) noexcept
{
	return static_cast<OutType>(value);
}

template <typename OutType, typename InType>
typename std::enable_if<std::is_floating_point<InType>::value && std::is_floating_point<OutType>::value, OutType>::type round(InType value, bool performRounding = true)  noexcept
{
	return performRounding ? static_cast<OutType>(::round(value)) : static_cast<OutType>(value);
}

template <typename OutType, typename InType>
constexpr typename std::enable_if<std::is_integral<OutType>::value && std::is_floating_point<InType>::value, OutType>::type round(InType value)  noexcept
{
	// The offset carries the sign of the value: rounding half away from zero, as ::round does
	return OutType(value >= InType(0) ? value + InType(0.5) : value - InType(0.5));
}

template <typename OutType, typename InType>
constexpr typename std::enable_if_t<std::is_integral<InType>::value, OutType> ceil(InType value) noexcept
{
	return static_cast<OutType>(value);
}

template <typename OutType, typename InType>
typename std::enable_if_t<std::is_floating_point<InType>::value, OutType> ceil(InType value) noexcept
{
	return static_cast<OutType>(::ceil(value));
}

// Integer abs
template <typename T>
constexpr typename std::enable_if<std::is_integral<T>::value, T>::type abs(T value) noexcept
{
	if constexpr (std::is_unsigned_v<T>)
		return value;
	else if (value > std::numeric_limits<T>::min())
		return value >= 0 ? value : -value;
	else // The most negative value has no positive counterpart: saturate rather than overflow
		return std::numeric_limits<T>::max();
}

// Floating-point abs
template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type abs(T value) noexcept
{
	return ::fabs(value);
}

namespace detail {

template <typename Comparator, typename T, typename... Rest>
[[nodiscard]] constexpr T extremum(const Comparator isBetter, const T first, const Rest... rest) noexcept
{
	static_assert((is_value_preserving_conversion_v<T, Rest> && ...), "Every argument must convert to the type of the first one without losing value");

	T result = first;
	pack::for_value([&](const T value) {
		if (isBetter(value, result))
			result = value;
	}, static_cast<T>(rest)...);

	return result;
}

} // namespace detail

// minimum and maximum return the type of the first argument and convert the rest to it.
// The other arguments must convert to it without losing value: cast explicitly for a lossy conversion.
// Ties keep the leftmost argument.

template <typename T, typename... Rest>
[[nodiscard]] constexpr T maximum(const T first, const Rest... rest) noexcept
{
	return detail::extremum([](const T left, const T right) { return left > right; }, first, rest...);
}

template <typename T, typename... Rest>
[[nodiscard]] constexpr T minimum(const T first, const Rest... rest) noexcept
{
	return detail::extremum([](const T left, const T right) { return left < right; }, first, rest...);
}

template<typename T>
constexpr T clamp(T lowerBoundary, T value, T upperBoundary) noexcept
{
	return value < lowerBoundary ? lowerBoundary : (value > upperBoundary ? upperBoundary : value);
}

template <typename T>
constexpr T signum(T value) noexcept
{
	if constexpr (std::is_unsigned_v<T>)
		return value > 0 ? T(1) : T(0);
	else
		return (value > 0) ? T(1) : ((value < 0) ? T(-1) : T(0));
}

template <typename T, typename ResultType = T>
constexpr ResultType squared(T value) noexcept
{
	return (ResultType)value * (ResultType)value;
}

// Both bounds are inclusive. The arguments need not share a type: a bool result needs no common type, unlike minimum and maximum.
template <typename T, typename LowerBound, typename UpperBound>
constexpr bool isInRange(const T value, const LowerBound lowerBound, const UpperBound upperBound) noexcept
{
	// The built-in operators would convert the signed operand to unsigned; std::cmp_* compares the actual values
	if constexpr (is_standard_integer_v<T> && is_standard_integer_v<LowerBound> && is_standard_integer_v<UpperBound>)
		return std::cmp_greater_equal(value, lowerBound) && std::cmp_less_equal(value, upperBound);
	else
		return value >= lowerBound && value <= upperBound;
}

template <typename ResultType, typename... Args>
[[nodiscard]] constexpr ResultType arithmeticMean(Args&&... args) noexcept
{
	static_assert(sizeof...(Args) > 0, "The mean of no values is undefined");

	ResultType acc = ResultType(0);
	constexpr size_t n = sizeof...(Args);

	pack::for_value([&](auto&& value) {
		acc += value;
	}, std::forward<Args>(args)...);

	// n is converted to ResultType: dividing a signed accumulator by size_t would convert it to unsigned first
	return acc / ResultType(n);
}

template <typename ResultType, typename... Args>
[[nodiscard]] constexpr ResultType geometricMean(Args&&... args) noexcept
{
	static_assert(sizeof...(Args) > 0, "The mean of no values is undefined");

	ResultType acc = ResultType(1);
	constexpr size_t n = sizeof...(Args);

	pack::for_value([&](auto&& value) {
		acc *= value;
	}, std::forward<Args>(args)...);

	return (ResultType)pow(acc, 1.0 / (double)n);
}

[[nodiscard]] constexpr size_t pow2(size_t power) noexcept
{
	assert(power < sizeof(size_t) * 8);
	return size_t{1} << power;
}

[[nodiscard]] inline constexpr uint64_t reduce(uint32_t value, uint32_t range) noexcept
{
	return ((uint64_t)value * (uint64_t)range) >> 32;
}

namespace detail {

inline uint32_t fastmod_u32(uint32_t a, uint64_t M, uint32_t d) noexcept
{
	const uint64_t lowbits = M * a;
#if defined(__SIZEOF_INT128__)          // GCC / Clang
	return (uint32_t)(((__uint128_t)lowbits * d) >> 64);
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64))
	return (uint32_t)__umulh(lowbits, d);
#else                                   // portable: 32-bit targets
	const uint64_t lo = (lowbits & 0xFFFFFFFFull) * d;
	const uint64_t hi = (lowbits >> 32) * d;
	return (uint32_t)((hi + (lo >> 32)) >> 32);
#endif
}

inline constexpr uint64_t computeM_u32(uint32_t d) noexcept {
	assert(d != 0);
	return UINT64_C(0xFFFFFFFFFFFFFFFF) / d + 1;
}
} // namespace detail

struct FastMod32
{
	inline constexpr FastMod32(uint32_t divisor) noexcept
		: M(detail::computeM_u32(divisor)), d(divisor)
	{}

	[[nodiscard]] inline uint32_t mod(uint32_t a) const noexcept
	{
		return detail::fastmod_u32(a, M, d);
	}

private:
	uint64_t M;
	uint32_t d;
};

} // namespace Math
