#include "3rdparty/catch2/catch.hpp"
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
