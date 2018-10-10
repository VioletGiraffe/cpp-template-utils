#pragma once

#include <limits>
#include <random>

template <typename IntType>
class CRandomNumberGenerator
{
public:
	CRandomNumberGenerator(IntType seed = 0, IntType min = std::numeric_limits<IntType>::min(), IntType max = std::numeric_limits<IntType>::max()) : _rng(seed), _distribution(min, max)
	{}

	IntType rand()
	{
		return _distribution(_rng);
	}

private:
	std::mt19937_64 _rng;
	std::uniform_int_distribution<IntType> _distribution;
};
