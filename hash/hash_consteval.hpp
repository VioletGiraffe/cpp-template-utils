#pragma once

#include <string_view>
#include <stdint.h>

[[nodiscard]] consteval uint32_t murmur3_32_consteval(std::string_view s, uint32_t seed = 0) noexcept
{
	constexpr uint32_t c1 = 0xcc9e2d51u;
	constexpr uint32_t c2 = 0x1b873593u;

	uint32_t h = seed;
	const size_t nblocks = s.size() / 4;

	for (size_t i = 0; i < nblocks; ++i)
	{
		// A constant expression can't reinterpret 4 bytes as a uint32_t, so build the block by hand.
		// Fixed little-endian keeps results identical across platforms and matches the reference vectors.
		const size_t j = i * 4;
		uint32_t k = static_cast<uint32_t>(static_cast<uint8_t>(s[j]))
					 | static_cast<uint32_t>(static_cast<uint8_t>(s[j + 1])) << 8
					 | static_cast<uint32_t>(static_cast<uint8_t>(s[j + 2])) << 16
					 | static_cast<uint32_t>(static_cast<uint8_t>(s[j + 3])) << 24;

		k *= c1;
		k = (k << 15) | (k >> 17);
		k *= c2;

		h ^= k;
		h = (h << 13) | (h >> 19);
		h = h * 5u + 0xe6546b64u;
	}

	uint32_t k = 0;
	const size_t tail = nblocks * 4;
	switch (s.size() % 4)
	{
	case 3: k ^= static_cast<uint32_t>(static_cast<uint8_t>(s[tail + 2])) << 16; [[fallthrough]];
	case 2: k ^= static_cast<uint32_t>(static_cast<uint8_t>(s[tail + 1])) << 8; [[fallthrough]];
	case 1: k ^= static_cast<uint32_t>(static_cast<uint8_t>(s[tail]));
		k *= c1;
		k = (k << 15) | (k >> 17);
		k *= c2;
		h ^= k;
	}

	h ^= static_cast<uint32_t>(s.size());
	h ^= h >> 16;
	h *= 0x85ebca6bu;
	h ^= h >> 13;
	h *= 0xc2b2ae35u;
	h ^= h >> 16;

	return h;
}
