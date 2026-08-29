#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include "3rdparty/catch2/catch.hpp"
RESTORE_COMPILER_WARNINGS

#include "container/chunked_deque.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <iterator>
#include <random>
#include <string>
#include <vector>

// chunked_deque against std::deque. Every case is tagged [!benchmark], which Catch2 treats as hidden: the
// binary with no arguments runs the tests only, `tests "[!benchmark]"` runs these.
//
// Three element types, because MSVC's std::deque derives its block size from sizeof(T) and drops to a single
// element per heap block above 8 bytes:
//   uint64_t  - two elements per block, the least bad of the three for std::deque
//   payload64 - one block per element, and 64 bytes to move on every shift
//   string100 - one block per element, plus an allocation inside the element; cheap to move
//
// Random positions are drawn outside every measured region: these cases measure the container, not the RNG.

namespace {

// 64 bytes: past the threshold where MSVC's std::deque holds one element per heap block.
struct payload64
{
	payload64() noexcept = default;
	explicit payload64(uint64_t v) noexcept : value{ v } {}

	uint64_t value = 0;
	uint64_t rest[7] = {};
};

static_assert(sizeof(payload64) == 64);

// 100 characters: past every SSO buffer, so each element owns a heap allocation.
constexpr char longStringLiteral[] =
	"0123456789" "0123456789" "0123456789" "0123456789" "0123456789"
	"0123456789" "0123456789" "0123456789" "0123456789" "0123456789";

static_assert(sizeof(longStringLiteral) == 101);

[[nodiscard]] uint64_t valueOf(uint64_t item) noexcept { return item; }
[[nodiscard]] uint64_t valueOf(const payload64& item) noexcept { return item.value; }
// back(), not size(): reads through to the heap buffer instead of the string object.
[[nodiscard]] uint64_t valueOf(const std::string& item) noexcept { return static_cast<uint64_t>(item.back()); }

template <typename T> [[nodiscard]] T makeValue(size_t i);
template <> uint64_t makeValue<uint64_t>(size_t i) { return static_cast<uint64_t>(i); }
template <> payload64 makeValue<payload64>(size_t i) { return payload64{ static_cast<uint64_t>(i) }; }
template <> std::string makeValue<std::string>(size_t) { return std::string{ longStringLiteral }; }

template <typename T> struct typeLabel;
template <> struct typeLabel<uint64_t> { static constexpr const char* name = "uint64_t"; };
template <> struct typeLabel<payload64> { static constexpr const char* name = "payload64"; };
template <> struct typeLabel<std::string> { static constexpr const char* name = "string100"; };

template <typename Container> struct containerLabel;
template <typename T, size_t BlockSize> struct containerLabel<chunked_deque<T, BlockSize>> { static constexpr const char* name = "chunked_deque"; };
template <typename T, typename Allocator> struct containerLabel<std::deque<T, Allocator>> { static constexpr const char* name = "std::deque"; };

constexpr size_t elementCounts[] = { 50, 500, 1'000, 10'000, 100'000, 1'000'000 };

template <typename Container> constexpr bool isStdDeque = false;
template <typename T, typename Allocator> constexpr bool isStdDeque<std::deque<T, Allocator>> = true;

// Ceilings, per case and per container: only std::deque needs most of them, because it holds a heap block per
// element for the types over 8 bytes where chunked_deque allocates per block.
// Traversal and indexed read take none - they only read, and 1'000'000 is where they are worth seeing.

// Growth and the FIFO cycles.
template <typename Container>
constexpr size_t maxElementCountForPerElementWork = isStdDeque<Container> ? 100'000 : 1'000'000;

// Copy construction: both containers copy every element, and a string100 copy is an allocation of its own, so
// the ceiling is the cost of the case rather than of the container.
constexpr size_t maxElementCountForCopy = 100'000;

// Reaching an interior position is O(n) on both sides: std::deque shifts the tail, chunked_deque walks to it.
constexpr size_t maxElementCountForPositionedWork = 100'000;

// A pass erasing one element at a time is O(n^2) on std::deque, linear on chunked_deque.
template <typename Container>
constexpr size_t maxElementCountForErasePass = isStdDeque<Container> ? 10'000 : 1'000'000;

// Few enough that a positioned case leaves the container near the size it is named for.
[[nodiscard]] size_t positionedOperationCount(size_t elementCount) noexcept
{
	return std::min<size_t>(64, elementCount / 2);
}

template <typename Container>
void fill(Container& container, size_t elementCount)
{
	for (size_t i = 0; i < elementCount; ++i)
		container.push_back(makeValue<typename Container::value_type>(i));
}

[[nodiscard]] std::vector<size_t> randomPositions(size_t count, size_t lowest, size_t pastHighest)
{
	std::mt19937 rng{ 20260829u };
	std::vector<size_t> positions(count);
	for (size_t& position : positions)
		position = lowest + static_cast<size_t>(rng() % (pastHighest - lowest));

	return positions;
}

template <typename Container>
[[nodiscard]] std::string caseName(size_t elementCount)
{
	return std::string{ containerLabel<Container>::name } + ", " + typeLabel<typename Container::value_type>::name
		+ ", " + std::to_string(elementCount);
}

// Destruction is inside the measured region: both containers pay it, and for the types that allocate per
// element it is the other half of what this case exists to show.
template <typename Container>
void benchmarkGrowthBack()
{
	for (const size_t elementCount : elementCounts)
	{
		if (elementCount > maxElementCountForPerElementWork<Container>)
			continue;

		BENCHMARK_ADVANCED(caseName<Container>(elementCount))(Catch::Benchmark::Chronometer meter)
		{
			meter.measure([elementCount] {
				Container container;
				fill(container, elementCount);
				return container.size();
			});
		};
	}
}

template <typename Container>
void benchmarkGrowthFront()
{
	for (const size_t elementCount : elementCounts)
	{
		if (elementCount > maxElementCountForPerElementWork<Container>)
			continue;

		BENCHMARK_ADVANCED(caseName<Container>(elementCount))(Catch::Benchmark::Chronometer meter)
		{
			using T = typename Container::value_type;

			meter.measure([elementCount] {
				Container container;
				for (size_t i = 0; i < elementCount; ++i)
					container.push_front(makeValue<T>(i));

				return container.size();
			});
		};
	}
}

// Steady state: the container ends every run at the size it started, so all samples measure the same work.
template <typename Container>
void benchmarkFifoBack()
{
	for (const size_t elementCount : elementCounts)
	{
		if (elementCount > maxElementCountForPerElementWork<Container>)
			continue;

		BENCHMARK_ADVANCED(caseName<Container>(elementCount))(Catch::Benchmark::Chronometer meter)
		{
			using T = typename Container::value_type;

			Container container;
			fill(container, elementCount);

			meter.measure([&container, elementCount] {
				for (size_t i = 0; i < elementCount; ++i)
				{
					container.push_back(makeValue<T>(i));
					container.pop_front();
				}

				return container.size();
			});
		};
	}
}

template <typename Container>
void benchmarkFifoFront()
{
	for (const size_t elementCount : elementCounts)
	{
		if (elementCount > maxElementCountForPerElementWork<Container>)
			continue;

		BENCHMARK_ADVANCED(caseName<Container>(elementCount))(Catch::Benchmark::Chronometer meter)
		{
			using T = typename Container::value_type;

			Container container;
			fill(container, elementCount);

			meter.measure([&container, elementCount] {
				for (size_t i = 0; i < elementCount; ++i)
				{
					container.push_front(makeValue<T>(i));
					container.pop_back();
				}

				return container.size();
			});
		};
	}
}

template <typename Container>
void benchmarkTraversal()
{
	for (const size_t elementCount : elementCounts)
	{
		BENCHMARK_ADVANCED(caseName<Container>(elementCount))(Catch::Benchmark::Chronometer meter)
		{
			Container container;
			fill(container, elementCount);

			meter.measure([&container] {
				uint64_t sum = 0;
				for (const auto& item : container)
					sum += valueOf(item);

				return sum;
			});
		};
	}
}

template <typename Container>
void benchmarkCopyConstruction()
{
	for (const size_t elementCount : elementCounts)
	{
		if (elementCount > maxElementCountForCopy)
			continue;

		BENCHMARK_ADVANCED(caseName<Container>(elementCount))(Catch::Benchmark::Chronometer meter)
		{
			Container source;
			fill(source, elementCount);

			meter.measure([&source] {
				Container copy{ source };
				return copy.size();
			});
		};
	}
}

// chunked_deque counts live elements through the blocks to reach an index, std::deque computes it.
template <typename Container>
void benchmarkRandomIndexedRead()
{
	for (const size_t elementCount : elementCounts)
	{
		BENCHMARK_ADVANCED(caseName<Container>(elementCount))(Catch::Benchmark::Chronometer meter)
		{
			Container container;
			fill(container, elementCount);
			const std::vector<size_t> positions = randomPositions(positionedOperationCount(elementCount), 0, elementCount);

			meter.measure([&container, &positions] {
				uint64_t sum = 0;
				for (const size_t position : positions)
					sum += valueOf(container[position]);

				return sum;
			});
		};
	}
}

// The insertions grow the container, so the fill is rebuilt inside the measured region. Subtract the
// push_back growth case at the same size to isolate them.
// Positions exclude the front and the end: those are push_front and push_back, measured on their own.
template <typename Container>
void benchmarkRandomInsert()
{
	for (const size_t elementCount : elementCounts)
	{
		if (elementCount > maxElementCountForPositionedWork)
			continue;

		BENCHMARK_ADVANCED(caseName<Container>(elementCount))(Catch::Benchmark::Chronometer meter)
		{
			using T = typename Container::value_type;

			const std::vector<size_t> positions = randomPositions(positionedOperationCount(elementCount), 1, elementCount);

			meter.measure([&positions, elementCount] {
				Container container;
				fill(container, elementCount);

				for (const size_t position : positions)
					container.insert(std::next(container.cbegin(), static_cast<ptrdiff_t>(position)), makeValue<T>(position));

				return container.size();
			});
		};
	}
}

// The erasures consume the container, so the fill is rebuilt inside the measured region. Subtract the
// push_back growth case at the same size to isolate them.
template <typename Container>
void benchmarkRandomErase()
{
	for (const size_t elementCount : elementCounts)
	{
		if (elementCount > maxElementCountForPositionedWork)
			continue;

		BENCHMARK_ADVANCED(caseName<Container>(elementCount))(Catch::Benchmark::Chronometer meter)
		{
			const std::vector<size_t> positions = randomPositions(positionedOperationCount(elementCount), 0, elementCount);

			meter.measure([&positions, elementCount] {
				Container container;
				fill(container, elementCount);

				// The container shrinks as it goes, so a drawn position is folded into the current size.
				for (const size_t position : positions)
					container.erase(std::next(container.cbegin(), static_cast<ptrdiff_t>(position % container.size())));

				return container.size();
			});
		};
	}
}

// The pass consumes the container, so the fill is rebuilt inside the measured region. Subtract the push_back
// growth case at the same size to isolate the erasures.
template <typename Container>
void benchmarkEraseEveryEighth()
{
	for (const size_t elementCount : elementCounts)
	{
		if (elementCount > maxElementCountForErasePass<Container>)
			continue;

		BENCHMARK_ADVANCED(caseName<Container>(elementCount))(Catch::Benchmark::Chronometer meter)
		{
			meter.measure([elementCount] {
				Container container;
				fill(container, elementCount);

				size_t index = 0;
				for (auto it = container.begin(); it != container.end(); ++index)
				{
					if (index % 8 == 0)
						it = container.erase(it);
					else
						++it;
				}

				return container.size();
			});
		};
	}
}

}

// The container-and-type set lives here alone, so a fourth type is one line rather than one per case.
#define RUN_FOR_EVERY_CONTAINER(driver) \
	driver<chunked_deque<uint64_t>>(); \
	driver<std::deque<uint64_t>>(); \
	driver<chunked_deque<payload64>>(); \
	driver<std::deque<payload64>>(); \
	driver<chunked_deque<std::string>>(); \
	driver<std::deque<std::string>>()

TEST_CASE("Benchmark - push_back growth", "[chunked_deque][!benchmark]")
{
	RUN_FOR_EVERY_CONTAINER(benchmarkGrowthBack);
}

TEST_CASE("Benchmark - push_front growth", "[chunked_deque][!benchmark]")
{
	RUN_FOR_EVERY_CONTAINER(benchmarkGrowthFront);
}

TEST_CASE("Benchmark - push_back + pop_front", "[chunked_deque][!benchmark]")
{
	RUN_FOR_EVERY_CONTAINER(benchmarkFifoBack);
}

TEST_CASE("Benchmark - push_front + pop_back", "[chunked_deque][!benchmark]")
{
	RUN_FOR_EVERY_CONTAINER(benchmarkFifoFront);
}

TEST_CASE("Benchmark - traversal, begin to end", "[chunked_deque][!benchmark]")
{
	RUN_FOR_EVERY_CONTAINER(benchmarkTraversal);
}

TEST_CASE("Benchmark - copy construction", "[chunked_deque][!benchmark]")
{
	RUN_FOR_EVERY_CONTAINER(benchmarkCopyConstruction);
}

TEST_CASE("Benchmark - random indexed read", "[chunked_deque][!benchmark]")
{
	RUN_FOR_EVERY_CONTAINER(benchmarkRandomIndexedRead);
}

TEST_CASE("Benchmark - random insert", "[chunked_deque][!benchmark]")
{
	RUN_FOR_EVERY_CONTAINER(benchmarkRandomInsert);
}

TEST_CASE("Benchmark - random erase", "[chunked_deque][!benchmark]")
{
	RUN_FOR_EVERY_CONTAINER(benchmarkRandomErase);
}

TEST_CASE("Benchmark - erase every 8th in one pass", "[chunked_deque][!benchmark]")
{
	RUN_FOR_EVERY_CONTAINER(benchmarkEraseEveryEighth);
}
