#pragma once

#include <stdint.h>

inline constexpr uint64_t operator "" _u64(unsigned long long int value)
{
	return static_cast<uint64_t>(value);
}

inline constexpr size_t operator "" _z(size_t value)
{
	return static_cast<uint64_t>(value);
}
