#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include "3rdparty/catch2/catch.hpp"
RESTORE_COMPILER_WARNINGS

#include "hash/wheathash.hpp"

#include <iterator> // std::size
#include <memory> // std::addressof
#include <set>
#include <unordered_set>
#include <vector>

#include <stdint.h>
#include <stdio.h>
#include <string.h> // memset

// These tests pin the wrappers and the internal consistency of the digest, not its agreement with upstream wheathash:
// no reference vectors are embedded. The hidden "[.wheathash_vectors]" case prints a table for that purpose.
// Any such table is little-endian only: the tail cases compose bytes through native integer loads.

namespace {

	// Position-dependent: a uniform fill would hide a wrong offset in the tail switch
	[[nodiscard]] std::vector<uint8_t> testPattern(size_t length)
	{
		std::vector<uint8_t> bytes(length);
		for (size_t i = 0; i < length; ++i)
			bytes[i] = static_cast<uint8_t>(i * 7 + 1);

		return bytes;
	}

	// Two full iterations of the 16-byte loop, plus every case of the tail switch
	constexpr size_t maxTestedLength = 33;

	constexpr uint64_t documentedDefaultSeed = 7733305894521163487ULL;

	struct Padded {
		uint8_t tag;
		uint32_t value;
	};

	static_assert(sizeof(Padded) > sizeof(uint8_t) + sizeof(uint32_t), "The padding this fixture exercises was optimized away");
}

TEST_CASE("wheathash: every tail case and the bulk loop produce a distinct digest", "[wheathash]") {
	const std::vector<uint8_t> pattern = testPattern(64);

	std::set<uint64_t> digests;
	for (size_t length = 0; length <= maxTestedLength; ++length)
		digests.insert(wheathash64(pattern.data(), length));

	// A repeat among 34 lengths means some tail case ignores its input or its length
	CHECK(digests.size() == maxTestedLength + 1);
}

TEST_CASE("wheathash: the unseeded overload applies the documented default seed", "[wheathash]") {
	const std::vector<uint8_t> pattern = testPattern(64);

	for (size_t length = 0; length <= maxTestedLength; ++length)
		CHECK(wheathash64(pattern.data(), length) == wheathash64(pattern.data(), length, documentedDefaultSeed));
}

TEST_CASE("wheathash: the seed changes the digest", "[wheathash]") {
	const std::vector<uint8_t> pattern = testPattern(64);

	constexpr size_t lengths[] = { 0, 1, 7, 15, 16, 33 };
	constexpr uint64_t seeds[] = { 0, 1, documentedDefaultSeed, ~uint64_t(0) };

	for (const size_t length: lengths)
	{
		std::set<uint64_t> digests;
		for (const uint64_t seed: seeds)
			digests.insert(wheathash64(pattern.data(), length, seed));

		CHECK(digests.size() == std::size(seeds));
	}
}

TEST_CASE("wheathash32 folds the 64-bit digest", "[wheathash]") {
	const std::vector<uint8_t> pattern = testPattern(64);

	for (size_t length = 0; length <= maxTestedLength; ++length)
	{
		const uint64_t full = wheathash64(pattern.data(), length);
		CHECK(wheathash32(pattern.data(), length) == static_cast<uint32_t>((full >> 32) ^ full));
	}
}

TEST_CASE("wheathash64v hashes exactly the object representation", "[wheathash]") {
	const uint64_t value = 0x0123456789ABCDEFULL;
	CHECK(wheathash64v(value) == wheathash64(std::addressof(value), sizeof(value)));
	CHECK(wheathash64v(value, 12345ULL) == wheathash64(std::addressof(value), sizeof(value), 12345ULL));

	const Padded object{ 7, 42 };
	CHECK(wheathash64v(object) == wheathash64(std::addressof(object), sizeof(object)));
	CHECK(wheathash64v(object, 12345ULL) == wheathash64(std::addressof(object), sizeof(object), 12345ULL));
}

TEST_CASE("wheathash64v: padding bytes are part of the digest", "[wheathash]") {
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
	CHECK(wheathash64v(zeroed) != wheathash64v(filled));
}

TEST_CASE("wheathash: every input bit reaches the digest", "[wheathash]") {
	const uint64_t base = 0x0F1E2D3C4B5A6978ULL;
	const uint64_t baseDigest = wheathash64v(base);

	for (int bit = 0; bit < 64; ++bit)
	{
		const uint64_t flipped = base ^ (uint64_t(1) << bit);
		CHECK(wheathash64v(flipped) != baseDigest);
	}
}

TEST_CASE("wheathash: distinct inputs do not collide in bulk", "[wheathash]") {
	constexpr size_t count = 100000;

	std::unordered_set<uint64_t> digests;
	digests.reserve(count);
	for (size_t i = 0; i < count; ++i)
		digests.insert(wheathash64v(i));

	// Catches gross breakage only: a dropped finalizer, or a digest that echoes its input
	CHECK(digests.size() == count);
}

// Prints a pasteable table of digests for lengths 0 through maxTestedLength. Run it against the upstream reference
// implementation to turn the checks above into fidelity tests. The leading dot in the tag keeps it out of normal runs.
TEST_CASE("wheathash: print known-answer vectors", "[.wheathash_vectors]") {
	const std::vector<uint8_t> pattern = testPattern(64);

	for (size_t length = 0; length <= maxTestedLength; ++length)
		printf("\t0x%016llXULL, // length %zu\n", static_cast<unsigned long long>(wheathash64(pattern.data(), length)), length);
}
