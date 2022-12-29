#include "3rdparty/catch2/catch.hpp"
#include "math/math.hpp"

TEST_CASE("pow2")
{
	uint64_t p = 2;
	for (uint64_t i = 1; i < 63; ++i, p *= 2)
		CHECK(Math::pow2(i) == p);

	static_assert(Math::pow2(16) == 65536);
}
