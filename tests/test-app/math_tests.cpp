#include "compiler/compiler_warnings_control.h"

#define CATCH_CONFIG_MAIN
DISABLE_COMPILER_WARNINGS
#include "3rdparty/catch2/catch.hpp"
RESTORE_COMPILER_WARNINGS

#include "math/math.hpp"

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
}
