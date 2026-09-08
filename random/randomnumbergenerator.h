#pragma once

#include "../hash/mixers.h"
#include "../utility/extra_type_traits.hpp"

#include <atomic>
#include <limits>
#include <random>
#include <stdint.h>

template <typename IntType, template <typename> class DistributionT = std::uniform_int_distribution, typename GeneratorT = std::mt19937_64>
class RandomNumberGenerator
{
	// std::uniform_int_distribution accepts only short, int, long, long long and their unsigned counterparts: bool, the character types and every one-byte integer are undefined for it
	static_assert(is_standard_integer_v<IntType> && sizeof(IntType) >= sizeof(short), "IntType must be an integer type no narrower than short");
public:
	RandomNumberGenerator(typename GeneratorT::result_type seed = 0, IntType min = (std::numeric_limits<IntType>::min)(), IntType max = (std::numeric_limits<IntType>::max)()) noexcept : _rng(seed), _distribution(min, max)
	{}

	IntType rand() noexcept
	{
		return _distribution(_rng);
	}

private:
	GeneratorT _rng;
	DistributionT<IntType> _distribution;
};

namespace rng_detail {

	// Every call returns a value no other call returns: one counter shared by every RNG specialization
	// The counter is scrambled: mt19937_64 correlates the streams of neighbouring seeds
	[[nodiscard]] inline uint64_t nextSeed() noexcept
	{
		static std::atomic<uint64_t> counter{ 0 };
		return mix_moremur(counter.fetch_add(1, std::memory_order_relaxed));
	}
} // namespace rng_detail

template <typename IntType, auto minValue = (std::numeric_limits<IntType>::min)(), auto maxValue = (std::numeric_limits<IntType>::max)(), template <typename> class DistributionT = std::uniform_int_distribution, typename GeneratorT = std::mt19937_64>
struct RNG {
	static IntType next() noexcept
	{
		using SeedType = typename GeneratorT::result_type;

		static thread_local RandomNumberGenerator<IntType, DistributionT, GeneratorT> rng{ static_cast<SeedType>(rng_detail::nextSeed()), minValue, maxValue };
		return rng.rand();
	}
};

using RandomChar = RNG<int16_t, 33, 126>;