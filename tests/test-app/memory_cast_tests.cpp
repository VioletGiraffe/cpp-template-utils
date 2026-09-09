#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include "3rdparty/catch2/catch.hpp"
RESTORE_COMPILER_WARNINGS

#include "utility/extra_type_traits.hpp"
#include "utility/memory_cast.hpp"

#include <algorithm>
#include <array>
#include <limits>
#include <memory>
#include <string>

#include <stdint.h>
#include <string.h>

namespace {

	struct Pair32 {
		uint32_t first;
		uint32_t second;
	};

	struct Padded {
		uint8_t tag;
		uint32_t value;
	};

	static_assert(sizeof(Pair32) == sizeof(uint64_t));
	static_assert(sizeof(Padded) > sizeof(uint8_t) + sizeof(uint32_t), "The padding this fixture exists to exercise was optimized away");

	template <typename T>
	[[nodiscard]] bool allBytesZero(const T& object) noexcept
	{
		const auto* const bytes = reinterpret_cast<const uint8_t*>(std::addressof(object));
		return std::all_of(bytes, bytes + sizeof(object), [](uint8_t byte) { return byte == 0; });
	}

	// MSVC reports a failing requires-expression outside a template as an error, so both probes are concepts

	template <typename Target, typename Source>
	concept castable_from_value = requires (const Source& source) { memory_cast<Target>(source); };

	template <typename Target, typename Source>
	concept castable_from_pointer = requires (Source* source) { memory_cast<Target>(source); };
}

// The value overload is constexpr, so a static_assert can only pass if that overload was the one selected
static_assert(memory_cast<double>(memory_cast<uint64_t>(2.5)) == 2.5);
static_assert(memory_cast<Pair32>(memory_cast<uint64_t>(Pair32{ 7, 9 })).second == 9);
static_assert(!std::numeric_limits<double>::is_iec559 || memory_cast<uint64_t>(1.0) == 0x3FF0000000000000ULL);

// The selector: is_trivially_serializable_v rejects pointers and arrays, so neither can reach the bit_cast overload
static_assert(is_trivially_serializable_v<double>);
static_assert(!is_trivially_serializable_v<uint8_t*>);
static_assert(!is_trivially_serializable_v<const uint8_t*>);
static_assert(!is_trivially_serializable_v<uint8_t[8]>);

static_assert(castable_from_pointer<uint64_t, const void>);
static_assert(castable_from_pointer<uint64_t, void>);
// Matches neither overload: the constraint rejects it, and it is not a pointer to deduce from
static_assert(!castable_from_value<uint64_t, std::string>);

// An unusable target, or a pointer to an unusable source, is diagnosed by a static_assert in the body rather than by
// a constraint, so it is a hard error and cannot be probed with requires.

TEST_CASE("memory_cast: value source round-trips", "[memory_cast]") {
	const double value = -1234.5678;
	CHECK(memory_cast<double>(memory_cast<uint64_t>(value)) == value);

	const Pair32 pair{ 0xDEADBEEF, 0x12345678 };
	const Pair32 restored = memory_cast<Pair32>(memory_cast<uint64_t>(pair));
	CHECK(restored.first == pair.first);
	CHECK(restored.second == pair.second);
}

TEST_CASE("memory_cast: reads through a const pointer", "[memory_cast]") {
	const uint64_t original = 0x0123456789ABCDEFULL;

	uint8_t buffer[sizeof(uint64_t)];
	::memcpy(buffer, std::addressof(original), sizeof(original));

	const uint8_t* const source = buffer;
	CHECK(memory_cast<uint64_t>(source) == original);
}

TEST_CASE("memory_cast: a non-const pointer reads the bytes, not the address", "[memory_cast]") {
	const uint64_t original = 0x0123456789ABCDEFULL;

	uint8_t buffer[sizeof(uint64_t)];
	::memcpy(buffer, std::addressof(original), sizeof(original));

	// std::bit_cast of the pointer itself would compile and yield the address on a 64-bit target
	uint8_t* const mutableSource = buffer;
	CHECK(memory_cast<uint64_t>(mutableSource) == original);
}

TEST_CASE("memory_cast: accepts a void pointer", "[memory_cast]") {
	const uint64_t original = 0xFEDCBA9876543210ULL;

	uint8_t buffer[sizeof(uint64_t)];
	::memcpy(buffer, std::addressof(original), sizeof(original));

	const void* const constSource = buffer;
	CHECK(memory_cast<uint64_t>(constSource) == original);

	void* const mutableSource = buffer;
	CHECK(memory_cast<uint64_t>(mutableSource) == original);
}

TEST_CASE("memory_cast: an array source decays to the pointer overload", "[memory_cast]") {
	const uint64_t original = 0x1122334455667788ULL;

	uint8_t buffer[sizeof(uint64_t)];
	::memcpy(buffer, std::addressof(original), sizeof(original));

	CHECK(memory_cast<uint64_t>(buffer) == original);
}

TEST_CASE("memory_cast: reads only sizeof(TargetType) out of a larger buffer", "[memory_cast]") {
	// The shape Sha3_Hasher::get64BitHash() uses: the first 8 bytes of a 32-byte hash
	std::array<uint8_t, 32> hash{};
	const uint64_t leading = 0xA1B2C3D4E5F60718ULL;
	::memcpy(hash.data(), std::addressof(leading), sizeof(leading));
	hash[sizeof(leading)] = 0xFF;

	CHECK(memory_cast<uint64_t>(hash.data()) == leading);
}

TEST_CASE("memory_cast: reads a struct out of raw memory", "[memory_cast]") {
	const Pair32 original{ 0x0A0B0C0D, 0x01020304 };

	uint8_t buffer[sizeof(Pair32)];
	::memcpy(buffer, std::addressof(original), sizeof(original));

	const Pair32 restored = memory_cast<Pair32>(buffer);
	CHECK(restored.first == original.first);
	CHECK(restored.second == original.second);
}

TEST_CASE("zero_object", "[memory_cast]") {
	SECTION("clears an object, padding included") {
		Padded object;
		::memset(std::addressof(object), 0xFF, sizeof(object));

		zero_object(object);

		CHECK(object.tag == 0);
		CHECK(object.value == 0);
		CHECK(allBytesZero(object));
	}

	SECTION("clears an array") {
		uint32_t values[4];
		::memset(values, 0xFF, sizeof(values));

		zero_object(values);

		CHECK(allBytesZero(values));
	}

	SECTION("clears a multidimensional array") {
		int matrix[3][4];
		::memset(matrix, 0xFF, sizeof(matrix));

		zero_object(matrix);

		CHECK(allBytesZero(matrix));
	}
}
