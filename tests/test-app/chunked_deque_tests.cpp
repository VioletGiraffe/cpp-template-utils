#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include "3rdparty/catch2/catch.hpp"
RESTORE_COMPILER_WARNINGS

#include "container/chunked_deque.hpp"

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <list>
#include <memory>
#include <random>
#include <ranges>
#include <stdexcept>
#include <vector>

// The default block size is otherwise never instantiated: every test names one explicitly.
static_assert(std::bidirectional_iterator<chunked_deque<int>::iterator>);
static_assert(std::bidirectional_iterator<chunked_deque<int>::const_iterator>);
static_assert(std::ranges::bidirectional_range<chunked_deque<int>>);
static_assert(std::ranges::bidirectional_range<const chunked_deque<int>>);

namespace {

	template <typename Deque>
	[[nodiscard]] std::vector<int> contents(const Deque& deque)
	{
		std::vector<int> values;
		for (const auto& item : deque)
			values.push_back(static_cast<int>(item));

		return values;
	}

	template <typename Deque>
	[[nodiscard]] std::vector<int> contentsReversed(const Deque& deque)
	{
		std::vector<int> values;
		for (auto it = deque.end(); it != deque.begin();)
			values.push_back(static_cast<int>(*--it));

		return values;
	}

	template <typename Deque>
	void fillBack(Deque& deque, std::initializer_list<int> values)
	{
		for (const int value : values)
			deque.push_back(value);
	}

	// Erases the first element equal to `value`. Returns the iterator erase() handed back.
	template <typename Deque>
	auto eraseValue(Deque& deque, int value)
	{
		const auto pos = std::find(deque.begin(), deque.end(), value);
		REQUIRE(pos != deque.end());
		return deque.erase(pos);
	}

	template <typename Deque>
	[[nodiscard]] auto iteratorAt(Deque& deque, size_t index)
	{
		return std::next(deque.begin(), static_cast<ptrdiff_t>(index));
	}

	// Counts live instances, so a leak or a double destruction shows up as a non-zero balance.
	struct tracked
	{
		static inline int liveCount = 0;
		static inline int moveCount = 0;

		int value = 0;

		tracked(int v = 0) noexcept : value{ v } { ++liveCount; }
		tracked(const tracked& other) noexcept : value{ other.value } { ++liveCount; }
		tracked(tracked&& other) noexcept : value{ other.value } { ++liveCount; ++moveCount; }
		tracked& operator=(const tracked& other) noexcept { value = other.value; return *this; }
		tracked& operator=(tracked&& other) noexcept { value = other.value; return *this; }
		~tracked() noexcept { --liveCount; }

		explicit operator int() const noexcept { return value; }

		static void reset() noexcept { liveCount = 0; moveCount = 0; }
	};

	struct move_only
	{
		std::unique_ptr<int> value;

		explicit move_only(int v) : value{ std::make_unique<int>(v) } {}
		move_only(move_only&&) noexcept = default;
		move_only& operator=(move_only&&) noexcept = default;

		explicit operator int() const noexcept { return *value; }
	};

	struct throws_on_nth
	{
		static inline int constructionsUntilThrow = -1;

		int value = 0;

		explicit throws_on_nth(int v) : value{ v }
		{
			if (constructionsUntilThrow == 0)
				throw std::runtime_error("construction");
			else if (constructionsUntilThrow > 0)
				--constructionsUntilThrow;
		}

		throws_on_nth(throws_on_nth&&) noexcept = default;
		throws_on_nth(const throws_on_nth&) = default;

		explicit operator int() const noexcept { return value; }
	};

	STORE_COMPILER_WARNINGS
	DISABLE_MSVC_WARNING(4324) // structure was padded due to alignment specifier

	struct alignas(32) over_aligned
	{
		int value = 0;

		over_aligned(int v = 0) noexcept : value{ v } {}
		explicit operator int() const noexcept { return value; }
	};

	RESTORE_COMPILER_WARNINGS

	// Every operation is applied to both the container and a std::list holding the same sequence, and the two
	// are compared after each one. Fixed seed: a failure has to be reproducible.
	template <size_t BlockSize>
	void compareAgainstList(unsigned seed, int operationCount)
	{
		chunked_deque<int, BlockSize> subject;
		std::list<int> oracle;
		std::mt19937 rng{ seed };
		int nextValue = 0;

		const auto randomIndex = [&rng](size_t size) { return static_cast<size_t>(rng() % size); };

		for (int operation = 0; operation < operationCount; ++operation)
		{
			const unsigned choice = rng() % 100;
			CAPTURE(BlockSize, seed, operation, choice, oracle.size());

			if (choice < 25)
			{
				subject.push_back(nextValue);
				oracle.push_back(nextValue);
				++nextValue;
			}
			else if (choice < 45)
			{
				subject.push_front(nextValue);
				oracle.push_front(nextValue);
				++nextValue;
			}
			else if (choice < 62 && !oracle.empty())
			{
				subject.pop_front();
				oracle.pop_front();
			}
			else if (choice < 74 && !oracle.empty())
			{
				subject.pop_back();
				oracle.pop_back();
			}
			else if (choice < 86)
			{
				const size_t index = oracle.empty() ? 0 : randomIndex(oracle.size() + 1);
				subject.insert(iteratorAt(subject, index), nextValue);
				oracle.insert(std::next(oracle.begin(), static_cast<ptrdiff_t>(index)), nextValue);
				++nextValue;
			}
			else if (choice < 96 && !oracle.empty())
			{
				const size_t index = randomIndex(oracle.size());
				subject.erase(iteratorAt(subject, index));
				oracle.erase(std::next(oracle.begin(), static_cast<ptrdiff_t>(index)));
			}
			else if (choice < 99)
			{
				const auto isEven = [](const int value) noexcept { return value % 2 == 0; };
				subject.remove_if(isEven);
				oracle.remove_if(isEven);
			}
			else
			{
				subject.clear();
				oracle.clear();
			}

			REQUIRE(subject.size() == oracle.size());
			REQUIRE(subject.empty() == oracle.empty());
			REQUIRE(contents(subject) == std::vector<int>(oracle.begin(), oracle.end()));
			if (!oracle.empty())
			{
				REQUIRE(subject.front() == oracle.front());
				REQUIRE(subject.back() == oracle.back());
			}
		}
	}
}

TEST_CASE("chunked_deque - an empty container", "[chunked_deque]")
{
	chunked_deque<int, 4> deque;

	CHECK(deque.size() == 0);
	CHECK(deque.empty());
	CHECK(deque.begin() == deque.end());
	CHECK(deque.cbegin() == deque.cend());
	CHECK(contents(deque).empty());

	deque.clear(); // Must be a no-op rather than touching absent blocks
	CHECK(deque.empty());
}

TEST_CASE("chunked_deque - push_back keeps insertion order across block boundaries", "[chunked_deque]")
{
	chunked_deque<int, 2> deque;
	fillBack(deque, { 1, 2, 3, 4, 5 });

	CHECK(deque.size() == 5);
	CHECK(contents(deque) == std::vector<int>{ 1, 2, 3, 4, 5 });
	CHECK(deque.front() == 1);
	CHECK(deque.back() == 5);
}

TEST_CASE("chunked_deque - push_front reverses, and mixes with push_back", "[chunked_deque]")
{
	SECTION("Front only")
	{
		chunked_deque<int, 2> deque;
		for (const int value : { 1, 2, 3, 4, 5 })
			deque.push_front(value);

		CHECK(contents(deque) == std::vector<int>{ 5, 4, 3, 2, 1 });
	}

	SECTION("The first element of an empty container takes slot 0 whichever end pushed it")
	{
		chunked_deque<int, 4> deque;
		deque.push_front(1);
		fillBack(deque, { 2, 3 });
		deque.push_front(0);

		CHECK(contents(deque) == std::vector<int>{ 0, 1, 2, 3 });
	}

	SECTION("Alternating ends")
	{
		chunked_deque<int, 3> deque;
		for (int i = 1; i <= 6; ++i)
		{
			deque.push_back(i);
			deque.push_front(-i);
		}

		CHECK(contents(deque) == std::vector<int>{ -6, -5, -4, -3, -2, -1, 1, 2, 3, 4, 5, 6 });
	}
}

TEST_CASE("chunked_deque - popping from both ends", "[chunked_deque]")
{
	chunked_deque<int, 2> deque;
	fillBack(deque, { 1, 2, 3, 4, 5 });

	deque.pop_front();
	deque.pop_back();
	CHECK(contents(deque) == std::vector<int>{ 2, 3, 4 });

	while (!deque.empty())
		deque.pop_front();

	CHECK(deque.empty());
	CHECK(deque.begin() == deque.end());

	// Re-anchored after emptying, so the container has to work from scratch
	fillBack(deque, { 7, 8, 9 });
	CHECK(contents(deque) == std::vector<int>{ 7, 8, 9 });
}

TEST_CASE("chunked_deque - a drained container re-anchors instead of accumulating blocks", "[chunked_deque]")
{
	chunked_deque<int, 1> deque; // One element per block: every push and pop crosses a boundary

	for (int round = 0; round < 200; ++round)
	{
		deque.push_back(round);
		deque.pop_front();
		REQUIRE(deque.empty());
	}

	fillBack(deque, { 1, 2, 3 });
	CHECK(contents(deque) == std::vector<int>{ 1, 2, 3 });
}

TEST_CASE("chunked_deque - erasure leaves the other elements where they are", "[chunked_deque]")
{
	chunked_deque<int, 4> deque;
	fillBack(deque, { 1, 2, 3, 4, 5, 6, 7, 8 });

	const int* const firstAddress = &*deque.begin();
	const int* const lastAddress = &*iteratorAt(deque, 7);

	eraseValue(deque, 3);
	eraseValue(deque, 6);

	CHECK(contents(deque) == std::vector<int>{ 1, 2, 4, 5, 7, 8 });
	CHECK(&*deque.begin() == firstAddress);
	CHECK(&*iteratorAt(deque, 5) == lastAddress);
}

TEST_CASE("chunked_deque - a push never fills a gap left by an erasure", "[chunked_deque]")
{
	SECTION("A gap between two live elements stays a gap")
	{
		// The property that separates this from a container which recycles erased slots: order stays exact.
		chunked_deque<int, 4> deque;
		fillBack(deque, { 1, 2, 3, 4 });
		eraseValue(deque, 2);

		deque.push_back(5);
		CHECK(contents(deque) == std::vector<int>{ 1, 3, 4, 5 });

		deque.push_front(0);
		CHECK(contents(deque) == std::vector<int>{ 0, 1, 3, 4, 5 });
	}

	SECTION("A slot erased at the front of its block is reused, because a push_front belongs there anyway")
	{
		chunked_deque<int, 4> deque;
		fillBack(deque, { 1, 2, 3, 4 });
		eraseValue(deque, 1); // Frees slot 0, which is below every remaining element

		deque.push_front(0);
		CHECK(contents(deque) == std::vector<int>{ 0, 2, 3, 4 });
	}

	SECTION("So is a slot erased at the back of its block")
	{
		chunked_deque<int, 4> deque;
		fillBack(deque, { 1, 2, 3, 4 });
		eraseValue(deque, 4);

		deque.push_back(5);
		CHECK(contents(deque) == std::vector<int>{ 1, 2, 3, 5 });
	}
}

TEST_CASE("chunked_deque - end() survives everything that adds or drops a block", "[chunked_deque]")
{
	chunked_deque<int, 2> deque;
	const auto endOfEmpty = deque.end();
	CHECK(deque.begin() == endOfEmpty);

	fillBack(deque, { 1, 2, 3, 4, 5, 6, 7 }); // Several blocks appended, and the pointer map grown
	CHECK(deque.end() == endOfEmpty);
	CHECK(deque.begin() != endOfEmpty);

	deque.push_front(0);                      // Prepends a block, renumbering every other one
	CHECK(deque.end() == endOfEmpty);

	deque.insert(iteratorAt(deque, 3), 9);    // Splits a block
	CHECK(deque.end() == endOfEmpty);

	while (!deque.empty())
		deque.pop_front();                    // Releases them again, one at a time

	CHECK(deque.end() == endOfEmpty);
	CHECK(deque.begin() == endOfEmpty);
}

TEST_CASE("chunked_deque - elements keep their addresses as the container grows", "[chunked_deque]")
{
	chunked_deque<int, 4> deque;
	fillBack(deque, { 1, 2, 3, 4, 5 });

	std::vector<const int*> addresses;
	for (const int& value : deque)
		addresses.push_back(&value);

	// Enough blocks to grow the pointer map several times, and enough prepends to walk its head around the ring
	for (int value = 6; value <= 2000; ++value)
		deque.push_back(value);
	for (int value = 0; value > -500; --value)
		deque.push_front(value);

	REQUIRE(deque.size() == 2500);

	auto it = std::next(deque.begin(), 500); // Past the prepended elements, at the original first one
	for (size_t index = 0; index < addresses.size(); ++index, ++it)
	{
		CAPTURE(index);
		CHECK(&*it == addresses[index]);
		CHECK(*addresses[index] == static_cast<int>(index) + 1);
	}
}

TEST_CASE("chunked_deque - the emplace family returns the element it created", "[chunked_deque]")
{
	chunked_deque<int, 2> deque;
	fillBack(deque, { 1, 2, 3 });

	int& atBack = deque.emplace_back(4);
	CHECK(&atBack == &deque.back());

	int& atFront = deque.emplace_front(0);
	CHECK(&atFront == &deque.front());

	chunked_deque<int, 2> single;
	single.push_back(42);
	CHECK(&single.front() == &single.back());
}

TEST_CASE("chunked_deque - erase returns the following element", "[chunked_deque]")
{
	chunked_deque<int, 2> deque;
	fillBack(deque, { 1, 2, 3, 4, 5 });

	SECTION("From the middle")
	{
		const auto next = eraseValue(deque, 3);
		REQUIRE(next != deque.end());
		CHECK(*next == 4);
	}

	SECTION("Across a block boundary")
	{
		const auto next = eraseValue(deque, 2); // Last slot of its block, so the successor is in the next one
		REQUIRE(next != deque.end());
		CHECK(*next == 3);
	}

	SECTION("The last element yields end()")
	{
		const auto next = eraseValue(deque, 5);
		CHECK(next == deque.end());
	}

	SECTION("Erasing the only element yields end()")
	{
		chunked_deque<int, 2> single;
		single.push_back(42);

		const auto endBeforeErasing = single.end();
		const auto next = single.erase(single.begin());
		CHECK(next == single.end());
		CHECK(next == endBeforeErasing); // Emptying the container released its block, and end() still matches
		CHECK(single.empty());
	}
}

TEST_CASE("chunked_deque - the ends skip and release blocks an erasure emptied", "[chunked_deque]")
{
	SECTION("Leading blocks")
	{
		chunked_deque<int, 1> deque;
		fillBack(deque, { 1, 2, 3, 4 });
		eraseValue(deque, 1);
		eraseValue(deque, 2);
		eraseValue(deque, 3);

		CHECK(deque.front() == 4);          // Scans past three empty blocks
		CHECK(*deque.begin() == 4);
		deque.pop_front();                  // Releases them
		CHECK(deque.empty());
	}

	SECTION("Trailing blocks")
	{
		chunked_deque<int, 1> deque;
		fillBack(deque, { 1, 2, 3, 4 });
		eraseValue(deque, 4);
		eraseValue(deque, 3);
		eraseValue(deque, 2);

		CHECK(deque.back() == 1);
		deque.pop_back();
		CHECK(deque.empty());
	}

	SECTION("A push_back reuses an emptied back block rather than allocating")
	{
		chunked_deque<int, 2> deque;
		fillBack(deque, { 1, 2, 3, 4 });
		eraseValue(deque, 3);
		eraseValue(deque, 4); // The second block is now empty but still present

		deque.push_back(5);
		CHECK(contents(deque) == std::vector<int>{ 1, 2, 5 });
	}

	SECTION("A push_front after the front block was emptied")
	{
		chunked_deque<int, 2> deque;
		fillBack(deque, { 1, 2, 3, 4 });
		eraseValue(deque, 1);
		eraseValue(deque, 2);

		deque.push_front(0);
		CHECK(contents(deque) == std::vector<int>{ 0, 3, 4 });
	}
}

TEST_CASE("chunked_deque - remove_if", "[chunked_deque]")
{
	chunked_deque<int, 3> deque;
	fillBack(deque, { 1, 2, 3, 4, 5, 6, 7, 8, 9 });

	SECTION("Removes nothing when the predicate never matches")
	{
		CHECK(deque.remove_if([](int) { return false; }) == 0);
		CHECK(contents(deque) == std::vector<int>{ 1, 2, 3, 4, 5, 6, 7, 8, 9 });
	}

	SECTION("Removes a scattered subset")
	{
		CHECK(deque.remove_if([](const int value) { return value % 2 == 0; }) == 4);
		CHECK(contents(deque) == std::vector<int>{ 1, 3, 5, 7, 9 });
		CHECK(deque.size() == 5);
	}

	SECTION("Removing everything empties and re-anchors the container")
	{
		CHECK(deque.remove_if([](int) { return true; }) == 9);
		CHECK(deque.empty());
		CHECK(deque.begin() == deque.end());

		fillBack(deque, { 1, 2 });
		CHECK(contents(deque) == std::vector<int>{ 1, 2 });
	}

	SECTION("Whole interior blocks may be emptied")
	{
		CHECK(deque.remove_if([](const int value) { return value >= 4 && value <= 6; }) == 3);
		CHECK(contents(deque) == std::vector<int>{ 1, 2, 3, 7, 8, 9 });
		CHECK(contentsReversed(deque) == std::vector<int>{ 9, 8, 7, 3, 2, 1 });
	}
}

TEST_CASE("chunked_deque - insert reaches each of its placements", "[chunked_deque]")
{
	SECTION("At the ends")
	{
		chunked_deque<int, 4> deque;
		fillBack(deque, { 2, 3 });

		const auto first = deque.insert(deque.cbegin(), 1);
		CHECK(*first == 1);
		const auto last = deque.insert(deque.cend(), 4);
		CHECK(*last == 4);
		CHECK(contents(deque) == std::vector<int>{ 1, 2, 3, 4 });
	}

	SECTION("Into a gap between two live slots of one block")
	{
		chunked_deque<int, 4> deque;
		fillBack(deque, { 1, 2, 3, 4 });
		eraseValue(deque, 2);
		eraseValue(deque, 3);

		const auto inserted = deque.insert(iteratorAt(deque, 1), 9); // Before 4, gap at slots 1 and 2
		CHECK(*inserted == 9);
		CHECK(contents(deque) == std::vector<int>{ 1, 9, 4 });
	}

	SECTION("Below the first live slot of the following block")
	{
		chunked_deque<int, 4> deque;
		fillBack(deque, { 1, 2, 3, 4, 5, 6, 7, 8 });
		eraseValue(deque, 5);
		eraseValue(deque, 6); // Second block holds 7, 8 at slots 2 and 3

		deque.insert(iteratorAt(deque, 4), 9); // Before 7
		CHECK(contents(deque) == std::vector<int>{ 1, 2, 3, 4, 9, 7, 8 });
	}

	SECTION("Above the last live slot of the preceding block")
	{
		chunked_deque<int, 4> deque;
		fillBack(deque, { 1, 2, 3, 4, 5 });
		eraseValue(deque, 4); // First block holds 1, 2, 3; the second holds 5 at slot 0

		deque.insert(iteratorAt(deque, 3), 9); // Before 5, no room below it, room above 3
		CHECK(contents(deque) == std::vector<int>{ 1, 2, 3, 9, 5 });
	}

	SECTION("Splitting a full block")
	{
		chunked_deque<int, 2> deque;
		fillBack(deque, { 1, 2, 3, 4 }); // Both blocks full

		const auto inserted = deque.insert(iteratorAt(deque, 1), 9); // Between 1 and 2, which are adjacent slots
		CHECK(contents(deque) == std::vector<int>{ 1, 9, 2, 3, 4 });
		CHECK(deque.size() == 5);
		CHECK(*inserted == 9);
		CHECK(std::distance(deque.begin(), inserted) == 1); // The split must not have moved the returned position
	}

	SECTION("Splitting at the first slot of a block")
	{
		chunked_deque<int, 2> deque;
		fillBack(deque, { 1, 2, 3, 4 });

		deque.insert(iteratorAt(deque, 2), 9); // Before 3, both neighbouring blocks full
		CHECK(contents(deque) == std::vector<int>{ 1, 2, 9, 3, 4 });
	}

	SECTION("Repeated insertion at the same point keeps splitting")
	{
		chunked_deque<int, 2> deque;
		fillBack(deque, { 1, 2, 3, 4 });
		for (int value = 10; value < 20; ++value)
			deque.insert(iteratorAt(deque, 2), value);

		CHECK(contents(deque) == std::vector<int>{ 1, 2, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 3, 4 });
		CHECK(deque.size() == 14);
	}

	SECTION("Into an empty container")
	{
		chunked_deque<int, 4> deque;
		deque.insert(deque.cbegin(), 1);
		CHECK(contents(deque) == std::vector<int>{ 1 });
	}
}

TEST_CASE("chunked_deque - iteration", "[chunked_deque]")
{
	chunked_deque<int, 3> deque;
	fillBack(deque, { 1, 2, 3, 4, 5, 6, 7 });

	SECTION("Forward and backward agree")
	{
		auto reversed = contentsReversed(deque);
		std::reverse(reversed.begin(), reversed.end());
		CHECK(reversed == contents(deque));
	}

	SECTION("A round trip through the sequence returns to the start")
	{
		auto it = deque.begin();
		std::advance(it, 7);
		CHECK(it == deque.end());
		std::advance(it, -7);
		CHECK(it == deque.begin());
		CHECK(*it == 1);
	}

	SECTION("std::distance counts the elements")
	{
		CHECK(std::distance(deque.begin(), deque.end()) == 7);
	}

	SECTION("A non-const iterator converts to a const one")
	{
		chunked_deque<int, 3>::const_iterator constant = deque.begin();
		CHECK(constant == deque.cbegin());
		CHECK(*constant == 1);
	}

	SECTION("Elements are mutable through a non-const iterator")
	{
		for (int& value : deque)
			value *= 2;

		CHECK(contents(deque) == std::vector<int>{ 2, 4, 6, 8, 10, 12, 14 });
	}

	SECTION("The container is iterable through a const reference")
	{
		const auto& constant = deque;
		CHECK(contents(constant) == std::vector<int>{ 1, 2, 3, 4, 5, 6, 7 });
		CHECK(constant.front() == 1);
		CHECK(constant.back() == 7);
	}
}

TEST_CASE("chunked_deque - block size edges", "[chunked_deque]")
{
	SECTION("One slot per block")
	{
		chunked_deque<int, 1> deque;
		fillBack(deque, { 1, 2, 3 });
		deque.push_front(0);
		CHECK(contents(deque) == std::vector<int>{ 0, 1, 2, 3 });

		deque.insert(iteratorAt(deque, 2), 9); // No room anywhere: every insertion splits
		CHECK(contents(deque) == std::vector<int>{ 0, 1, 9, 2, 3 });

		deque.pop_back();
		deque.pop_front();
		CHECK(contents(deque) == std::vector<int>{ 1, 9, 2 });
	}

	SECTION("A full 64-slot mask")
	{
		chunked_deque<int, 64> deque;
		for (int value = 0; value < 64; ++value)
			deque.push_back(value);

		CHECK(deque.size() == 64);
		CHECK(deque.back() == 63); // The highest bit of the mask

		deque.push_back(64); // Starts a second block
		CHECK(deque.back() == 64);

		deque.pop_back();
		CHECK(deque.back() == 63);

		deque.insert(iteratorAt(deque, 63), 99); // The first block is full, so this splits it
		CHECK(deque.size() == 65);
		CHECK(*iteratorAt(deque, 62) == 62);
		CHECK(*iteratorAt(deque, 63) == 99);
		CHECK(*iteratorAt(deque, 64) == 63);
	}
}

TEST_CASE("chunked_deque - the block map grows and wraps", "[chunked_deque]")
{
	chunked_deque<int, 1> deque; // One block per element, so the map is exercised hard

	// Cycling pushes and pops walks the map's head around the ring before it has to grow
	for (int round = 0; round < 40; ++round)
	{
		deque.push_back(round);
		if (round % 3 != 0)
			deque.pop_front();
	}

	for (int value = 100; value < 140; ++value)
		deque.push_back(value);

	// 40 pushes against 26 pops, and the pops take the oldest: the last 14 of the first run survive
	std::vector<int> expected;
	for (int round = 26; round < 40; ++round)
		expected.push_back(round);
	for (int value = 100; value < 140; ++value)
		expected.push_back(value);

	CHECK(contents(deque) == expected);
	CHECK(deque.size() == expected.size());
}

TEST_CASE("chunked_deque - copy, move and swap", "[chunked_deque]")
{
	chunked_deque<int, 3> source;
	fillBack(source, { 1, 2, 3, 4, 5 });

	SECTION("Copy construction")
	{
		chunked_deque<int, 3> copy{ source };
		CHECK(contents(copy) == contents(source));
		copy.push_back(6);
		CHECK(source.size() == 5); // Independent storage
	}

	SECTION("Copy assignment")
	{
		chunked_deque<int, 3> copy;
		fillBack(copy, { 9, 9, 9, 9, 9, 9, 9 });
		copy = source;
		CHECK(contents(copy) == std::vector<int>{ 1, 2, 3, 4, 5 });
	}

	SECTION("Self-assignment leaves the container intact")
	{
		const auto& self = source;
		source = self;
		CHECK(contents(source) == std::vector<int>{ 1, 2, 3, 4, 5 });
	}

	SECTION("Move construction empties the source")
	{
		chunked_deque<int, 3> moved{ std::move(source) };
		CHECK(contents(moved) == std::vector<int>{ 1, 2, 3, 4, 5 });
		CHECK(source.empty());
	}

	SECTION("Move assignment releases what the target held")
	{
		chunked_deque<int, 3> target;
		fillBack(target, { 7, 8 });
		target = std::move(source);
		CHECK(contents(target) == std::vector<int>{ 1, 2, 3, 4, 5 });
		CHECK(source.empty());
	}

	SECTION("Self-move-assignment keeps the elements")
	{
		auto* self = &source; // Through a pointer, or the compiler diagnoses the self-move at the call site
		source = std::move(*self);
		CHECK(contents(source) == std::vector<int>{ 1, 2, 3, 4, 5 });
	}

	SECTION("Swap")
	{
		chunked_deque<int, 3> other;
		fillBack(other, { 7, 8 });
		source.swap(other);
		CHECK(contents(source) == std::vector<int>{ 7, 8 });
		CHECK(contents(other) == std::vector<int>{ 1, 2, 3, 4, 5 });
	}
}

TEST_CASE("chunked_deque - element lifetime", "[chunked_deque]")
{
	tracked::reset();

	SECTION("Everything is destroyed with the container")
	{
		{
			chunked_deque<tracked, 4> deque;
			for (int value = 0; value < 20; ++value)
				deque.push_back(tracked{ value });

			CHECK(tracked::liveCount == 20);
		}

		CHECK(tracked::liveCount == 0);
	}

	SECTION("clear() destroys everything and leaves a usable container")
	{
		chunked_deque<tracked, 4> deque;
		for (int value = 0; value < 10; ++value)
			deque.emplace_back(value);

		deque.clear();
		CHECK(tracked::liveCount == 0);
		CHECK(deque.empty());

		deque.emplace_back(1);
		CHECK(tracked::liveCount == 1);
	}

	SECTION("Erasure and popping destroy exactly one element each")
	{
		chunked_deque<tracked, 3> deque;
		for (int value = 0; value < 9; ++value)
			deque.emplace_back(value);

		deque.pop_front();
		deque.pop_back();
		deque.erase(iteratorAt(deque, 3));
		CHECK(tracked::liveCount == 6);
		CHECK(deque.size() == 6);

		deque.remove_if([](const tracked& item) { return item.value % 2 == 0; });
		CHECK(tracked::liveCount == static_cast<int>(deque.size()));
	}

	SECTION("A split moves the block's tail once per element")
	{
		chunked_deque<tracked, 4> deque;
		for (int value = 0; value < 4; ++value)
			deque.emplace_back(value);

		tracked::moveCount = 0;
		deque.insert(iteratorAt(deque, 1), tracked{ 9 });
		CHECK(tracked::moveCount <= 4); // Three relocated, plus the inserted one
		CHECK(tracked::liveCount == 5);
		CHECK(contents(deque) == std::vector<int>{ 0, 9, 1, 2, 3 });
	}

	CHECK(tracked::liveCount == 0);
}

TEST_CASE("chunked_deque - a move-only element type", "[chunked_deque]")
{
	chunked_deque<move_only, 2> deque;
	for (int value = 0; value < 5; ++value)
		deque.push_back(move_only{ value });

	deque.emplace_front(99);
	CHECK(deque.size() == 6);
	CHECK(contents(deque) == std::vector<int>{ 99, 0, 1, 2, 3, 4 });

	deque.erase(iteratorAt(deque, 2));
	CHECK(contents(deque) == std::vector<int>{ 99, 0, 2, 3, 4 });

	chunked_deque<move_only, 2> moved{ std::move(deque) };
	CHECK(moved.size() == 5);
}

TEST_CASE("chunked_deque - a throwing constructor leaves the container unchanged", "[chunked_deque]")
{
	chunked_deque<throws_on_nth, 2> deque;
	throws_on_nth::constructionsUntilThrow = -1;
	for (int value = 0; value < 4; ++value)
		deque.emplace_back(value);

	SECTION("Mid-block")
	{
		throws_on_nth::constructionsUntilThrow = 0;
		CHECK_THROWS_AS(deque.emplace_back(9), std::runtime_error);
	}

	SECTION("When a new block is needed")
	{
		deque.emplace_back(4); // Fills the last block... or starts one; either way the next push allocates
		deque.emplace_back(5);
		throws_on_nth::constructionsUntilThrow = 0;
		CHECK_THROWS_AS(deque.emplace_back(9), std::runtime_error);
	}

	SECTION("From an insertion that had already split the block")
	{
		throws_on_nth::constructionsUntilThrow = 0;
		CHECK_THROWS_AS(deque.emplace(iteratorAt(deque, 1), 9), std::runtime_error);
		CHECK(deque.size() == 4);
		CHECK(contents(deque) == std::vector<int>{ 0, 1, 2, 3 }); // The split moved elements but changed no order
	}

	throws_on_nth::constructionsUntilThrow = -1;
	const size_t sizeAfterThrow = deque.size();
	deque.emplace_back(100);
	CHECK(deque.size() == sizeAfterThrow + 1);
	CHECK(deque.back().value == 100);
}

TEST_CASE("chunked_deque - over-aligned elements", "[chunked_deque]")
{
	chunked_deque<over_aligned, 4> deque;
	for (int value = 0; value < 10; ++value)
		deque.push_back(over_aligned{ value });

	for (const over_aligned& item : deque)
		CHECK(reinterpret_cast<uintptr_t>(&item) % alignof(over_aligned) == 0);

	CHECK(contents(deque) == std::vector<int>{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 });
}

TEST_CASE("chunked_deque - randomized comparison against std::list", "[chunked_deque]")
{
	// Block sizes chosen for the edges (1, 64) and for values that are neither powers of two nor divisors of
	// the operation counts, so boundaries land at varying offsets.
	for (unsigned seed = 1; seed <= 4; ++seed)
	{
		compareAgainstList<1>(seed, 300);
		compareAgainstList<2>(seed, 300);
		compareAgainstList<3>(seed, 400);
		compareAgainstList<8>(seed, 500);
		compareAgainstList<64>(seed, 500);
	}
}
