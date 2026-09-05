#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include "3rdparty/catch2/catch.hpp"
RESTORE_COMPILER_WARNINGS

#include "container/set_operations.hpp"

#include <set>
#include <string>
#include <vector>

TEST_CASE("longestCommonStart", "[set_operations]")
{
	using Strings = std::vector<std::string>;

	CHECK(SetOperations::longestCommonStart(Strings{}).empty());
	CHECK(SetOperations::longestCommonStart(Strings{ "abc" }) == "abc");
	CHECK(SetOperations::longestCommonStart(Strings{ "abc", "abc" }) == "abc");
	CHECK(SetOperations::longestCommonStart(Strings{ "abcd", "abxx", "abcz" }) == "ab");
	CHECK(SetOperations::longestCommonStart(Strings{ "abc", "xyz" }).empty());
	CHECK(SetOperations::longestCommonStart(Strings{ "abc", "abd", "xyz" }).empty());

	// A subset that is a prefix of the others is the answer, whatever its position
	CHECK(SetOperations::longestCommonStart(Strings{ "abcdef", "abc" }) == "abc");
	CHECK(SetOperations::longestCommonStart(Strings{ "abc", "abcdef" }) == "abc");
	CHECK(SetOperations::longestCommonStart(Strings{ "abcdef", "abcd", "abc" }) == "abc");
	CHECK(SetOperations::longestCommonStart(Strings{ "abc", "abcd", "abcdef" }) == "abc");
	CHECK(SetOperations::longestCommonStart(Strings{ "abcd", "abc", "abxx" }) == "ab");

	// An empty subset leaves nothing in common
	CHECK(SetOperations::longestCommonStart(Strings{ "abc", "" }).empty());
	CHECK(SetOperations::longestCommonStart(Strings{ "", "abc" }).empty());
	CHECK(SetOperations::longestCommonStart(Strings{ "", "" }).empty());

	using IntSequences = std::vector<std::vector<int>>;

	CHECK(SetOperations::longestCommonStart(IntSequences{ { 1, 2, 3 }, { 1, 2 } }) == std::vector<int>{ 1, 2 });
	CHECK(SetOperations::longestCommonStart(IntSequences{ { 1, 2, 3 }, { 1, 2, 3 } }) == std::vector<int>{ 1, 2, 3 });
	CHECK(SetOperations::longestCommonStart(IntSequences{ { 1, 2, 3 }, { 4, 5 } }).empty());
}

TEST_CASE("calculateDiff", "[set_operations]")
{
	using Ints = std::vector<int>;

	SECTION("no duplicates")
	{
		const auto overlapping = SetOperations::calculateDiff(Ints{ 1, 2, 3 }, Ints{ 2, 3, 4 });
		CHECK(overlapping.elements_from_a_not_in_b == Ints{ 1 });
		CHECK(overlapping.elements_from_b_not_in_a == Ints{ 4 });
		CHECK(overlapping.common_elements == Ints{ 2, 3 });

		const auto disjoint = SetOperations::calculateDiff(Ints{ 1, 2 }, Ints{ 3, 4 });
		CHECK(disjoint.elements_from_a_not_in_b == Ints{ 1, 2 });
		CHECK(disjoint.elements_from_b_not_in_a == Ints{ 3, 4 });
		CHECK(disjoint.common_elements.empty());

		const auto identical = SetOperations::calculateDiff(Ints{ 1, 2 }, Ints{ 1, 2 });
		CHECK(identical.elements_from_a_not_in_b.empty());
		CHECK(identical.elements_from_b_not_in_a.empty());
		CHECK(identical.common_elements == Ints{ 1, 2 });

		const auto empty = SetOperations::calculateDiff(Ints{}, Ints{});
		CHECK(empty.elements_from_a_not_in_b.empty());
		CHECK(empty.elements_from_b_not_in_a.empty());
		CHECK(empty.common_elements.empty());
	}

	SECTION("every bucket holds each item once")
	{
		// b's second occurrence matches the same single item in a
		const auto duplicateInB = SetOperations::calculateDiff(Ints{ 1 }, Ints{ 1, 1 });
		CHECK(duplicateInB.common_elements == Ints{ 1 });
		CHECK(duplicateInB.elements_from_a_not_in_b.empty());
		CHECK(duplicateInB.elements_from_b_not_in_a.empty());

		const auto duplicateInA = SetOperations::calculateDiff(Ints{ 1, 1 }, Ints{});
		CHECK(duplicateInA.elements_from_a_not_in_b == Ints{ 1 });

		const auto both = SetOperations::calculateDiff(Ints{ 1, 1, 2 }, Ints{ 2, 2, 3 });
		CHECK(both.elements_from_a_not_in_b == Ints{ 1 });
		CHECK(both.elements_from_b_not_in_a == Ints{ 3 });
		CHECK(both.common_elements == Ints{ 2 });

		// The common bucket no longer depends on which side the duplicates are on
		CHECK(SetOperations::calculateDiff(Ints{ 1, 1 }, Ints{ 1 }).common_elements
			== SetOperations::calculateDiff(Ints{ 1 }, Ints{ 1, 1 }).common_elements);
	}

	SECTION("input order is preserved")
	{
		const auto diff = SetOperations::calculateDiff(Ints{ 3, 1, 2 }, Ints{});
		CHECK(diff.elements_from_a_not_in_b == Ints{ 3, 1, 2 });

		const auto reordered = SetOperations::calculateDiff(Ints{}, Ints{ 3, 1, 2, 3 });
		CHECK(reordered.elements_from_b_not_in_a == Ints{ 3, 1, 2 });
	}

	SECTION("mixed container types")
	{
		// std::set takes the member find and the reference-returning uniqueElements overload
		const auto diff = SetOperations::calculateDiff(std::set<int>{ 1, 2 }, Ints{ 2, 3 });
		CHECK(diff.elements_from_a_not_in_b == Ints{ 1 });
		CHECK(diff.elements_from_b_not_in_a == Ints{ 3 });
		CHECK(diff.common_elements == Ints{ 2 });
	}
}
