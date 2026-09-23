#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include "3rdparty/catch2/catch.hpp"
RESTORE_COMPILER_WARNINGS

#include "utility/heap_optional.hpp"

#include <string>
#include <utility>

static_assert(sizeof(heap_optional<std::string>) == sizeof(void*));

TEST_CASE("heap_optional starts empty and holds an assigned value", "[heap_optional]")
{
	heap_optional<std::string> value;
	CHECK_FALSE(value);
	CHECK_FALSE(value.has_value());

	value = std::string{ "held" };
	REQUIRE(value);
	CHECK(*value == "held");
	CHECK(value->size() == 4);

	value = std::nullopt;
	CHECK_FALSE(value);

	value = std::string{ "again" };
	value.reset();
	CHECK_FALSE(value);

	const heap_optional<std::string> fromNullopt{ std::nullopt };
	CHECK_FALSE(fromNullopt);
}

TEST_CASE("heap_optional copies deep-copy the value", "[heap_optional]")
{
	const heap_optional<std::string> original{ std::string{ "original" } };
	heap_optional<std::string> copy{ original };
	REQUIRE(copy);
	CHECK(&*copy != &*original);
	*copy = "changed";
	CHECK(*original == "original");

	heap_optional<std::string> assigned;
	assigned = original;
	REQUIRE(assigned);
	CHECK(&*assigned != &*original);
	CHECK(*assigned == "original");

	const heap_optional<std::string> empty;
	assigned = empty;
	CHECK_FALSE(assigned);
}

TEST_CASE("heap_optional assignment reuses a held allocation", "[heap_optional]")
{
	heap_optional<std::string> value{ std::string{ "first" } };
	const std::string* const storage = &*value;

	value = std::string{ "second" };
	CHECK(&*value == storage);

	const heap_optional<std::string> other{ std::string{ "third" } };
	value = other;
	CHECK(&*value == storage);
	CHECK(*value == "third");
}

TEST_CASE("heap_optional moves transfer the allocation and leave the source empty", "[heap_optional]")
{
	heap_optional<std::string> source{ std::string{ "moved" } };
	const std::string* const storage = &*source;

	heap_optional<std::string> destination{ std::move(source) };
	CHECK_FALSE(source);
	REQUIRE(destination);
	CHECK(&*destination == storage);

	heap_optional<std::string> assigned;
	assigned = std::move(destination);
	CHECK_FALSE(destination);
	REQUIRE(assigned);
	CHECK(&*assigned == storage);
}

TEST_CASE("heap_optional equality compares values, not allocations", "[heap_optional]")
{
	const heap_optional<std::string> emptyA, emptyB;
	const heap_optional<std::string> a{ std::string{ "a" } }, alsoA{ std::string{ "a" } }, b{ std::string{ "b" } };

	CHECK(emptyA == emptyB);
	CHECK(a == alsoA);
	CHECK(a != b);
	CHECK(a != emptyA);
	CHECK(emptyA != a);
}
