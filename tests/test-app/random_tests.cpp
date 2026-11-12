#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include "3rdparty/catch2/catch.hpp"
RESTORE_COMPILER_WARNINGS

#include "random/randomnumbergenerator.h"

#include <set>
#include <thread>
#include <vector>

#include <stdint.h>

// next() can throw only when its seed function can
static_assert(noexcept(RNG<int>::next()));
static_assert(!noexcept(EntropySeededRNG<int>::next()));

namespace {

	// The first values a facade draws on a new thread, whose generator is seeded by that first draw
	template <typename Facade>
	[[nodiscard]] std::vector<uint64_t> firstDrawsOnNewThread(size_t count)
	{
		std::vector<uint64_t> values;
		std::thread{ [&] {
			for (size_t i = 0; i < count; ++i)
				values.push_back(static_cast<uint64_t>(Facade::next()));
		} }.join();
		return values;
	}

} // namespace

TEMPLATE_TEST_CASE("RNG facades draw within their bounds and reach every value of a narrow range", "[random]", (RNG<int, 3, 7>), (EntropySeededRNG<int, 3, 7>))
{
	std::set<int> drawn;
	for (int i = 0; i < 1000; ++i)
	{
		const int value = TestType::next();
		REQUIRE(value >= 3);
		REQUIRE(value <= 7);
		drawn.insert(value);
	}
	CHECK(drawn.size() == 5); // A value is missed with probability (4/5)^1000
}

TEMPLATE_TEST_CASE("RNG facades give each thread its own sequence", "[random]", RNG<uint64_t>, EntropySeededRNG<uint64_t>)
{
	CHECK(firstDrawsOnNewThread<TestType>(4) != firstDrawsOnNewThread<TestType>(4));
}

TEST_CASE("EntropySeededRNG seeds differ on every call, so processes draw different sequences", "[random]")
{
	CHECK(rng_detail::entropySeed() != rng_detail::entropySeed()); // Equal with probability 2^-64
}

TEST_CASE("EntropySeededRNG spreads over a range reaching past 2^63", "[random]")
{
	// A 64-bit bounded draw can collapse to a constant for such a range: Qt's QRandomGenerator::bounded() does
	std::set<uint64_t> drawn;
	size_t aboveHalf = 0;
	for (int i = 0; i < 100; ++i)
	{
		const uint64_t value = EntropySeededRNG<uint64_t, 1>::next();
		REQUIRE(value >= 1);
		drawn.insert(value);
		if (value > (uint64_t{ 1 } << 63))
			++aboveHalf;
	}
	CHECK(drawn.size() == 100);
	CHECK(aboveHalf > 0); // None with probability 2^-100
}
