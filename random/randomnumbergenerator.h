#pragma once

#include <limits>
#include <random>
#include <type_traits>

template <typename IntType, template <typename> class DistributionT = std::uniform_int_distribution, typename GeneratorT = std::mt19937_64>
class RandomNumberGenerator
{
	static_assert (std::is_integral_v<IntType>, "IntType must be integral.");
public:
	constexpr RandomNumberGenerator(IntType seed = 0, IntType min = std::numeric_limits<IntType>::min(), IntType max = std::numeric_limits<IntType>::max()) noexcept : _rng(seed), _distribution(min, max)
	{}

	IntType rand() noexcept
	{
		return _distribution(_rng);
	}

private:
	GeneratorT _rng;
	DistributionT<IntType> _distribution;
};
