#include "compiler/compiler_warnings_control.h"

#define CATCH_CONFIG_MAIN
DISABLE_COMPILER_WARNINGS
#include "3rdparty/catch2/catch.hpp"
RESTORE_COMPILER_WARNINGS

#include "math/math.hpp"

#include <cmath>

TEST_CASE("pow2", "[math]")
{
	static_assert(Math::pow2(0) == 1);
	static_assert(Math::pow2(1) == 2);
	static_assert(Math::pow2(16) == 65536);

	size_t p = 1;
	for (size_t i = 0; i < sizeof(size_t) * 8; ++i, p *= 2)
		CHECK(Math::pow2(i) == p);
}

TEST_CASE("abs", "[math]")
{
	static_assert(Math::abs(0) == 0);
	static_assert(Math::abs(5) == 5);
	static_assert(Math::abs(-5) == 5);
	static_assert(Math::abs(int64_t{-5}) == 5);

	// Unsigned values are their own absolute value, zero included
	static_assert(Math::abs(0u) == 0u);
	static_assert(Math::abs(7u) == 7u);
	static_assert(Math::abs(std::numeric_limits<uint64_t>::max()) == std::numeric_limits<uint64_t>::max());

	// The most negative value saturates
	static_assert(Math::abs(std::numeric_limits<int32_t>::min()) == std::numeric_limits<int32_t>::max());
	static_assert(Math::abs(std::numeric_limits<int64_t>::min()) == std::numeric_limits<int64_t>::max());
	static_assert(Math::abs(std::numeric_limits<int32_t>::min() + 1) == std::numeric_limits<int32_t>::max());

	CHECK(Math::abs(-1.5) == 1.5);
	CHECK(Math::abs(1.5f) == 1.5f);
	CHECK(std::signbit(Math::abs(-0.0)) == false);
}

TEST_CASE("round and floor to a number of decimal digits", "[math]")
{
	CHECK(Math::round(1.27, 1) == Approx(1.3));
	CHECK(Math::round(1.24, 1) == Approx(1.2));
	CHECK(Math::round(2.0, 3) == Approx(2.0));
	CHECK(Math::round(1.27f, 1) == Approx(1.3f));

	// Negative values round to the nearest, not towards zero
	CHECK(Math::round(-1.27, 1) == Approx(-1.3));
	CHECK(Math::round(-1.24, 1) == Approx(-1.2));

	// Ties go away from zero on both sides
	CHECK(Math::round(1.25, 1) == Approx(1.3));
	CHECK(Math::round(-1.25, 1) == Approx(-1.3));

	// Magnitudes past the range of int64_t
	CHECK(Math::round(1.5e19, 0) == Approx(1.5e19));
	CHECK(Math::floor(-1.5e19, 0) == Approx(-1.5e19));

	CHECK(Math::floor(1.29, 1) == Approx(1.2));
	CHECK(Math::floor(-1.0, 2) == Approx(-1.0));

	// floor goes towards negative infinity, not towards zero
	CHECK(Math::floor(-1.21, 1) == Approx(-1.3));
	CHECK(Math::floor(-1.29, 1) == Approx(-1.3));

	// A negative digit count rounds to tens, hundreds and so on
	CHECK(Math::round(1234.0, -2) == Approx(1200.0));
	CHECK(Math::floor(1299.0, -2) == Approx(1200.0));

	// Past the largest power of ten a double holds exactly
	CHECK(Math::round(1.5, 30) == Approx(1.5));
}

TEST_CASE("round and floor to an integral type", "[math]")
{
	static_assert(Math::round<int>(1.4) == 1);
	static_assert(Math::round<int>(1.5) == 2);
	static_assert(Math::round<int>(1.7) == 2);
	static_assert(Math::round<int>(0.0) == 0);

	// Half away from zero on the negative side too
	static_assert(Math::round<int>(-1.4) == -1);
	static_assert(Math::round<int>(-1.5) == -2);
	static_assert(Math::round<int>(-1.7) == -2);
	static_assert(Math::round<int64_t>(-2.5) == -3);

	static_assert(Math::round<int>(42) == 42);
	static_assert(Math::round<int64_t>(-42) == -42);

	CHECK(Math::round<float>(-1.6) == -2.0f);
	CHECK(Math::round<float>(-1.6, false) == -1.6f);

	// floor goes towards negative infinity where round goes to the nearest
	CHECK(Math::floor<int>(1.9) == 1);
	CHECK(Math::floor<int>(-1.2) == -2);
	static_assert(Math::floor<int>(42) == 42);
}

TEST_CASE("ceil", "[math]")
{
	CHECK(Math::ceil<int>(1.0) == 1);
	CHECK(Math::ceil<int>(1.1) == 2);
	CHECK(Math::ceil<int>(-1.9) == -1);
	CHECK(Math::ceil<int64_t>(-0.5) == 0);
	CHECK(Math::ceil<double>(2.5) == 3.0);

	static_assert(Math::ceil<int>(42) == 42);
	static_assert(Math::ceil<int64_t>(-42) == -42);
}

TEST_CASE("signum", "[math]")
{
	static_assert(Math::signum(5) == 1);
	static_assert(Math::signum(-5) == -1);
	static_assert(Math::signum(0) == 0);

	static_assert(Math::signum(7u) == 1u);
	static_assert(Math::signum(0u) == 0u);

	static_assert(Math::signum(-0.5) == -1.0);
	static_assert(Math::signum(0.0) == 0.0);
	static_assert(Math::signum(1e300) == 1.0);
}

TEST_CASE("isInRange", "[math]")
{
	static_assert(Math::isInRange(5, 0, 10));
	static_assert(!Math::isInRange(11, 0, 10));
	static_assert(!Math::isInRange(-1, 0, 10));

	// Both bounds are inclusive
	static_assert(Math::isInRange(0, 0, 10));
	static_assert(Math::isInRange(10, 0, 10));

	// Mixed signedness compares by value, where the built-in operators would convert the signed operand to unsigned
	static_assert(Math::isInRange(size_t{5}, 0, 10));
	static_assert(!Math::isInRange(-1, 0u, 10u));
	static_assert(!Math::isInRange(int64_t{-1}, size_t{0}, size_t{10}));
	static_assert(!Math::isInRange(std::numeric_limits<uint64_t>::max(), 0, 10));

	// The types std::cmp_* rejects fall back to the built-in operators
	static_assert(Math::isInRange('c', 'a', 'z'));
	static_assert(!Math::isInRange('A', 'a', 'z'));
	static_assert(Math::isInRange(true, false, true));

	static_assert(Math::isInRange(3.5, 0, 10));
	static_assert(!Math::isInRange(10.5, 0, 10));
}

TEST_CASE("FastMod32", "[math]")
{
	static_assert(std::is_copy_assignable_v<Math::FastMod32>);

	for (const uint32_t divisor : { 1u, 2u, 3u, 7u, 64u, 1000u, 0x7FFFFFFFu, 0xFFFFFFFFu })
	{
		const Math::FastMod32 fastMod{ divisor };
		for (const uint32_t value : { 0u, 1u, 2u, 63u, 1000u, 123456789u, 0xFFFFFFFEu, 0xFFFFFFFFu })
			CHECK(fastMod.mod(value) == value % divisor);
	}
}

TEST_CASE("arithmeticMean", "[math]")
{
	REQUIRE(Math::arithmeticMean<int>(2, 4, 6) == 4);
	REQUIRE(Math::arithmeticMean<float>(2, 1) == 1.5f);

	static_assert(Math::arithmeticMean<int>(99, 1, 0, 4) == 26);

	CHECK(Math::arithmeticMean<int>(5) == 5);

	// A negative sum divided by the argument count: the count must not drag the division into unsigned arithmetic
	CHECK(Math::arithmeticMean<int>(-4, -2) == -3);
	CHECK(Math::arithmeticMean<int>(-10, 2) == -4);
	CHECK(Math::arithmeticMean<float>(-2, -1) == -1.5f);
	CHECK(Math::arithmeticMean<int64_t>(-9, -9, -9) == -9);
	static_assert(Math::arithmeticMean<int>(-99, -1, 0, -4) == -26);

	// Integral result types truncate towards zero
	CHECK(Math::arithmeticMean<int>(1, 2) == 1);
	CHECK(Math::arithmeticMean<int>(-1, -2) == -1);

	CHECK(Math::arithmeticMean<double>(1, 2.5f, 4) == 2.5);
}

TEST_CASE("geometricMean", "[math]")
{
	CHECK(Math::geometricMean<double>(4) == Approx(4.0));
	CHECK(Math::geometricMean<double>(2, 8) == Approx(4.0));
	CHECK(Math::geometricMean<double>(1, 2, 4) == Approx(2.0));
	CHECK(Math::geometricMean<float>(3.0f, 12.0f) == Approx(6.0f));
}

TEST_CASE("minimum and maximum", "[math]")
{
	CHECK(Math::maximum(42) == 42);
	CHECK(Math::minimum(42) == 42);

	CHECK(Math::maximum(1, 7, 3) == 7);
	CHECK(Math::minimum(1, 7, 3) == 1);
	CHECK(Math::maximum(-1, -7, -3) == -1);
	CHECK(Math::minimum(-1, -7, -3) == -7);

	static_assert(Math::maximum(1, 2, 3, 4, 5, 4, 3, 2, 1) == 5);
	static_assert(Math::minimum(5, 4, 3, 2, 1, 2, 3, 4, 5) == 1);

	// The first argument's type is the result type
	static_assert(std::is_same_v<float, decltype(Math::maximum(1.0f, 2))>);
	static_assert(std::is_same_v<int64_t, decltype(Math::minimum(int64_t{1}, int32_t{2}))>);
	static_assert(Math::maximum(1.0f, 3) == 3.0f);
	static_assert(Math::minimum(1.0, 2.0f, 3) == 1.0);

	static_assert(Math::maximum(uint8_t{1}, uint8_t{200}) == 200);
	static_assert(Math::maximum('a', 'z') == 'z');

	CHECK(Math::maximum(1.5, 1.5) == 1.5);
	CHECK(Math::minimum(std::numeric_limits<double>::lowest(), 0.0) == std::numeric_limits<double>::lowest());

	static_assert(Math::maximum(std::numeric_limits<uint32_t>::max(), uint32_t{0}) == std::numeric_limits<uint32_t>::max());
	static_assert(Math::minimum(std::numeric_limits<int64_t>::min(), int64_t{0}) == std::numeric_limits<int64_t>::min());

	// Ties keep the leftmost argument, observable only through the sign of zero
	CHECK(std::signbit(Math::minimum(0.0, -0.0)) == false);
	CHECK(std::signbit(Math::minimum(-0.0, 0.0)) == true);

	constexpr double infinity = std::numeric_limits<double>::infinity();
	CHECK(Math::maximum(1.0, infinity) == infinity);
	CHECK(Math::minimum(1.0, -infinity) == -infinity);

	// Every comparison against a NaN is false: a NaN argument is discarded, a NaN first argument is never replaced
	const double nan = std::numeric_limits<double>::quiet_NaN();
	CHECK(Math::maximum(1.0, nan, 2.0) == 2.0);
	CHECK(std::isnan(Math::minimum(nan, 1.0)));

	// int -> float is accepted although it loses precision beyond the mantissa width
	CHECK(Math::maximum(1.0f, 16777217) == 16777216.0f);
}
