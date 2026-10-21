#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include "3rdparty/catch2/catch.hpp"
RESTORE_COMPILER_WARNINGS

#include "hash/rapidhash.hpp"

#include <iterator> // std::size
#include <memory> // std::addressof
#include <set>
#include <unordered_set>
#include <vector>

#include <stdint.h>
#include <string.h> // memset

// These tests pin the wrappers and the internal consistency of the digest, not its agreement with upstream rapidhash.

namespace {

	// Position-dependent: a uniform fill would hide a wrong offset in a read
	[[nodiscard]] std::vector<uint8_t> testPattern(size_t length)
	{
		std::vector<uint8_t> bytes(length);
		for (size_t i = 0; i < length; ++i)
			bytes[i] = static_cast<uint8_t>(i * 7 + 1);

		return bytes;
	}

	// Past one full 112-byte block of the bulk loop, so every length branch runs
	constexpr size_t maxTestedLength = 130;

	struct Padded {
		uint8_t tag;
		uint32_t value;
	};

	static_assert(sizeof(Padded) > sizeof(uint8_t) + sizeof(uint32_t), "The padding this fixture exercises was optimized away");
}

TEST_CASE("rapidhash: every length branch and the bulk loop produce a distinct digest", "[rapidhash]") {
	const std::vector<uint8_t> pattern = testPattern(256);

	std::set<uint64_t> digests;
	for (size_t length = 0; length <= maxTestedLength; ++length)
		digests.insert(rapidhash64(pattern.data(), length));

	// A repeat means some branch ignores its input or its length
	CHECK(digests.size() == maxTestedLength + 1);
}

TEST_CASE("rapidhash: the unseeded overload applies seed 0", "[rapidhash]") {
	const std::vector<uint8_t> pattern = testPattern(256);

	for (size_t length = 0; length <= maxTestedLength; ++length)
		CHECK(rapidhash64(pattern.data(), length) == rapidhash64(pattern.data(), length, 0));
}

TEST_CASE("rapidhash: the seed changes the digest", "[rapidhash]") {
	const std::vector<uint8_t> pattern = testPattern(256);

	constexpr size_t lengths[] = { 0, 1, 4, 16, 17, 48, 112, 113, 130 };
	constexpr uint64_t seeds[] = { 0, 1, 0x0123456789ABCDEFULL, ~uint64_t(0) };

	for (const size_t length: lengths)
	{
		std::set<uint64_t> digests;
		for (const uint64_t seed: seeds)
			digests.insert(rapidhash64(pattern.data(), length, seed));

		CHECK(digests.size() == std::size(seeds));
	}
}

TEST_CASE("rapidhash64v hashes exactly the object representation", "[rapidhash]") {
	const uint64_t value = 0x0123456789ABCDEFULL;
	CHECK(rapidhash64v(value) == rapidhash64(std::addressof(value), sizeof(value)));
	CHECK(rapidhash64v(value, 12345ULL) == rapidhash64(std::addressof(value), sizeof(value), 12345ULL));

	const Padded object{ 7, 42 };
	CHECK(rapidhash64v(object) == rapidhash64(std::addressof(object), sizeof(object)));
	CHECK(rapidhash64v(object, 12345ULL) == rapidhash64(std::addressof(object), sizeof(object), 12345ULL));
}

TEST_CASE("rapidhash64v: padding bytes are part of the digest", "[rapidhash]") {
	Padded zeroed;
	::memset(std::addressof(zeroed), 0x00, sizeof(zeroed));
	zeroed.tag = 7;
	zeroed.value = 42;

	Padded filled;
	::memset(std::addressof(filled), 0xFF, sizeof(filled));
	filled.tag = 7;
	filled.value = 42;

	REQUIRE(zeroed.tag == filled.tag);
	REQUIRE(zeroed.value == filled.value);

	// The documented consequence of hashing the representation: equal members are not enough for equal digests
	CHECK(rapidhash64v(zeroed) != rapidhash64v(filled));
}

TEST_CASE("rapidhash: every input bit reaches the digest", "[rapidhash]") {
	const uint64_t base = 0x0F1E2D3C4B5A6978ULL;
	const uint64_t baseDigest = rapidhash64v(base);

	for (int bit = 0; bit < 64; ++bit)
	{
		const uint64_t flipped = base ^ (uint64_t(1) << bit);
		CHECK(rapidhash64v(flipped) != baseDigest);
	}
}

TEST_CASE("rapidhash: distinct inputs do not collide in bulk", "[rapidhash]") {
	constexpr size_t count = 100000;

	std::unordered_set<uint64_t> digests;
	digests.reserve(count);
	for (size_t i = 0; i < count; ++i)
		digests.insert(rapidhash64v(i));

	// Catches gross breakage only: a dropped finalizer, or a digest that echoes its input
	CHECK(digests.size() == count);
}
