#include "compiler/compiler_warnings_control.h"

#define CATCH_CONFIG_MAIN
DISABLE_COMPILER_WARNINGS
#include "3rdparty/catch2/catch.hpp"
RESTORE_COMPILER_WARNINGS

#include "math/math.hpp"

#include <cmath>

TEST_CASE("pow2", "[math]")
{
	uint64_t p = 2;
	for (uint64_t i = 1; i < 63; ++i, p *= 2)
		CHECK(Math::pow2(i) == p);

	static_assert(Math::pow2(16) == 65536);
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
