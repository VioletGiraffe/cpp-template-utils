#pragma once

#include <stdint.h>
#include <type_traits>

inline constexpr uint64_t operator "" _u64(unsigned long long int value)
{
	return static_cast<uint64_t>(value);
}
