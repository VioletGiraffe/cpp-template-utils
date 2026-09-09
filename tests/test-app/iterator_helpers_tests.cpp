#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include "3rdparty/catch2/catch.hpp"
RESTORE_COMPILER_WARNINGS

#include "container/iterator_helpers.hpp"

#include <algorithm>
#include <array>
#include <concepts>
#include <iterator>
#include <list>
#include <map>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

namespace {

	struct Point
	{
		int x = 0;
		int y = 0;
	};

	using VectorIterator = const_forward_iterator_wrapper<std::vector<int>>;
}

static_assert(std::forward_iterator<VectorIterator>);
static_assert(std::forward_iterator<const_forward_iterator_wrapper<std::list<int>>>);
static_assert(std::forward_iterator<const_forward_iterator_wrapper<std::set<int>>>);
static_assert(std::is_same_v<std::iterator_traits<VectorIterator>::value_type, int>);
static_assert(std::is_same_v<std::iterator_traits<VectorIterator>::reference, const int&>);

namespace {

	// MSVC reports a failing requires-expression outside a template as an error, so every probe below is a concept

	template <typename Iterator>
	concept has_arrow = requires (const Iterator& it) { it.operator->(); };

	template <typename Container>
	concept wraps_lvalue = requires (const Container& c) { forward_iterator_wrapper::cbegin(c); forward_iterator_wrapper::cend(c); };

	// Disjunction, so a negative result proves neither factory accepts a temporary
	template <typename Container>
	concept wraps_temporary = requires { forward_iterator_wrapper::cbegin(Container{}); } || requires { forward_iterator_wrapper::cend(Container{}); };
}

// operator-> exists only where the underlying iterator can supply the pointer
static_assert(has_arrow<VectorIterator>);
static_assert(!has_arrow<const_forward_iterator_wrapper<std::vector<bool>>>);

// A temporary cannot be bound: the wrapper would outlive the container
static_assert(std::constructible_from<VectorIterator, const std::vector<int>&, std::vector<int>::const_iterator>);
static_assert(!std::constructible_from<VectorIterator, std::vector<int>&&, std::vector<int>::const_iterator>);
static_assert(wraps_lvalue<std::vector<int>>);
static_assert(!wraps_temporary<std::vector<int>>);

TEST_CASE("const_forward_iterator_wrapper - traversal", "[iterator_helpers]")
{
	const std::vector<int> values{ 1, 2, 3 };

	auto it = forward_iterator_wrapper::cbegin(values);
	CHECK(it.isBound());
	CHECK_FALSE(it.endReached());
	CHECK(*it == 1);

	CHECK(*++it == 2);
	CHECK(*++it == 3);

	++it;
	CHECK(it.endReached());
	CHECK(it == values.cend());
	CHECK(it == forward_iterator_wrapper::cend(values));
}

TEST_CASE("const_forward_iterator_wrapper - an empty container", "[iterator_helpers]")
{
	const std::vector<int> empty;

	const auto it = forward_iterator_wrapper::cbegin(empty);
	CHECK(it.endReached());
	CHECK(it == forward_iterator_wrapper::cend(empty));
}

TEST_CASE("const_forward_iterator_wrapper - containers without random access", "[iterator_helpers]")
{
	SECTION("std::list")
	{
		const std::list<int> values{ 1, 2, 3 };

		int sum = 0;
		for (auto it = forward_iterator_wrapper::cbegin(values); !it.endReached(); ++it)
			sum += *it;

		CHECK(sum == 6);
	}

	SECTION("std::set")
	{
		const std::set<int> values{ 3, 1, 2 };

		std::vector<int> visited;
		for (auto it = forward_iterator_wrapper::cbegin(values); !it.endReached(); ++it)
			visited.push_back(*it);

		CHECK(visited == std::vector<int>{ 1, 2, 3 });
	}
}

TEST_CASE("const_forward_iterator_wrapper - operator->", "[iterator_helpers]")
{
	SECTION("an iterator that is a class")
	{
		const std::map<int, std::string> values{ { 1, "one" }, { 2, "two" } };

		const auto it = forward_iterator_wrapper::cbegin(values);
		CHECK(it->first == 1);
		CHECK(it->second == "one");
	}

	SECTION("an iterator that is a pointer")
	{
		const std::array<Point, 2> values{{ Point{ 1, 2 }, Point{ 3, 4 } }};

		const auto it = forward_iterator_wrapper::cbegin(values);
		CHECK(it->x == 1);
		CHECK(it->y == 2);
	}
}

TEST_CASE("const_forward_iterator_wrapper - a container with proxy references", "[iterator_helpers]")
{
	const std::vector<bool> values{ true, false };

	auto it = forward_iterator_wrapper::cbegin(values);
	CHECK(*it);
	++it;
	CHECK_FALSE(*it);
	++it;
	CHECK(it.endReached());
}

TEST_CASE("const_forward_iterator_wrapper - standard algorithms", "[iterator_helpers]")
{
	const std::vector<int> values{ 4, 8, 15, 16, 23, 42 };

	const auto begin = forward_iterator_wrapper::cbegin(values);
	const auto end = forward_iterator_wrapper::cend(values);

	CHECK(std::distance(begin, end) == 6);
	CHECK(std::count(begin, end, 15) == 1);

	const auto found = std::find(begin, end, 16);
	CHECK(found != end);
	CHECK(*found == 16);
	CHECK(std::find(begin, end, 100) == end);
}

TEST_CASE("const_forward_iterator_wrapper - post-increment returns the previous position", "[iterator_helpers]")
{
	const std::vector<int> values{ 1, 2 };

	auto it = forward_iterator_wrapper::cbegin(values);
	const auto previous = it++;

	CHECK(*previous == 1);
	CHECK(*it == 2);
	CHECK_FALSE(previous == it);
	CHECK(previous != it);
}

TEST_CASE("const_forward_iterator_wrapper - binding", "[iterator_helpers]")
{
	const VectorIterator unbound;
	CHECK_FALSE(unbound.isBound());

	const std::vector<int> values{ 1, 2, 3 };
	CHECK(forward_iterator_wrapper::cbegin(values).isBound());

	// Assignment moves within the container the wrapper is already bound to
	auto it = forward_iterator_wrapper::cbegin(values);
	it = values.cend();
	CHECK(it.isBound());
	CHECK(it.endReached());
}
