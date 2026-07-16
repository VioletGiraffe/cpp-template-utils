#pragma once

#include <stdint.h>

[[nodiscard]] inline uint64_t mix_moremur(uint64_t x)
{
	x ^= x >> 27;
	x *= 0x3C79AC492BA7B653ull;
	x ^= x >> 33;
	x *= 0x1C69B3F74AC4AE35ull;
	x ^= x >> 27;
	return x;
}
