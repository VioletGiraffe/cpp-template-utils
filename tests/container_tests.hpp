#pragma once

#include "catch2/catch.hpp"
#include "container/multi_index.hpp"

TEST_CASE("Construction", "[multiindex]") {
	struct S {
		int intValue;
		float floatValue;
	};

	try {
		MultiIndexSet<S, &S::intValue, &S::floatValue> set;

		static_assert(std::is_same_v<decltype(set)::PrimaryKeyType, int>);
		static_assert(std::is_same_v<decltype(set)::SecondaryKeyType, float>);
	} catch(...) {
		FAIL();
	}
}

TEST_CASE("Empty set tests", "[multiindex]") {
	struct S {
		int intValue;
		float floatValue;
	};

	try {
		MultiIndexSet<S, &S::intValue, &S::floatValue> set;
		REQUIRE_NOTHROW(set.size() == 0);
		REQUIRE_NOTHROW(set.empty());
		REQUIRE_NOTHROW(set.findPrimary(0) == set.end());
		REQUIRE_NOTHROW(set.findPrimary(10) == set.end());
		REQUIRE_NOTHROW(set.begin() == set.end());

		auto range = set.findSecondary(10);
		REQUIRE_NOTHROW(range.first == range.second);
		range = set.findSecondaryInRange(100.0f, 500.0f);
		REQUIRE_NOTHROW(range.first == range.second);
		REQUIRE_NOTHROW(std::distance(range.first, range.second) == 0);
	} catch(...) {
		FAIL();
	}
}

TEST_CASE("Compile time constant construction", "[multiindex]") {
	struct S {
		int intValue;
		float floatValue;
	};

	try {
		MultiIndexSet<S, &S::intValue, &S::floatValue> set {{100, 111.0f}, {0, 1.0f}, {10, 11.0f}};
		REQUIRE_NOTHROW(set.size() == 3);
		REQUIRE_NOTHROW(!set.empty());
		REQUIRE_NOTHROW(std::distance(set.begin(), set.end()) == 3);
		REQUIRE_NOTHROW(++++++set.begin() == set.end());

		REQUIRE_NOTHROW(set.begin()->floatValue == 1.0f && set.begin()->intValue == 0);
		REQUIRE_NOTHROW(((++set.begin())->floatValue == 11.0f) && ((++set.begin())->intValue == 10));
		REQUIRE_NOTHROW(((++++set.begin())->floatValue == 111.0f) && ((++++set.begin())->intValue == 100));
		REQUIRE_NOTHROW(((--set.end())->floatValue == 111.0f) && ((--set.end())->intValue == 100));
	} catch(...) {
		FAIL();
	}
}

TEST_CASE("Searching in MultiIndexSet", "[multiindex]") {
	struct S {
		int intValue;
		float floatValue;
	};

	try {
		MultiIndexSet<S, &S::intValue, &S::floatValue> set{ {100, 111.0f}, {0, 1.0f}, {10, 11.0f} };

		SECTION("By primary index")
		{
			REQUIRE_NOTHROW(set.findPrimary(0) != set.end());
			REQUIRE_NOTHROW(set.findPrimary(0) == set.begin());
			REQUIRE_NOTHROW(set.findPrimary(10) == ++set.begin());
			REQUIRE_NOTHROW(set.findPrimary(100) == ++++set.begin());
			REQUIRE_NOTHROW(set.findPrimary(-7) == set.end());
			REQUIRE_NOTHROW(set.findPrimary(10) != set.end());
			REQUIRE_NOTHROW(set.findPrimary(10.0f) == ++set.begin());
		}

		SECTION("By secondary index") {
			auto range = set.findSecondary(10);
			REQUIRE_NOTHROW(range.first == range.second);
			range = set.findSecondaryInRange(100.0f, 500.0f);
			REQUIRE_NOTHROW(range.first != range.second);
			REQUIRE_NOTHROW(std::distance(range.first, range.second) == 1);
			REQUIRE_NOTHROW((*range.first)->floatValue == 111.0f);

			range = set.findSecondaryInRange(1.0f, 11.0f);
			REQUIRE_NOTHROW(range.first != range.second);
			REQUIRE_NOTHROW(std::distance(range.first, range.second) == 1);
			REQUIRE_NOTHROW((*range.first)->floatValue == 1.0f);

			range = set.findSecondaryInRange(1.01f, 11.0f);
			REQUIRE_NOTHROW(range.first == range.second);
			REQUIRE_NOTHROW(std::distance(range.first, range.second) == 0);

			range = set.findSecondaryInRange(-1.01f, 1.0f);
			REQUIRE_NOTHROW(range.first == range.second);
			REQUIRE_NOTHROW(std::distance(range.first, range.second) == 0);

			range = set.findSecondaryInRange(1.0f, 11.01f);
			REQUIRE_NOTHROW(range.first != range.second);
			REQUIRE_NOTHROW(std::distance(range.first, range.second) == 2);
			REQUIRE_NOTHROW((*range.first)->floatValue == 1.0f);
			REQUIRE_NOTHROW((*++range.first)->floatValue == 11.0f);
		}
	}
	catch (...) {
		FAIL();
	}
}

TEST_CASE("Complex secondary lookup", "[multiindex]") {
	struct S {
		int intValue;
		float floatValue;
	};

	try {
		MultiIndexSet<S, &S::intValue, &S::floatValue> set{ {100, 111.0f}, {0, 1.0f}, {10, 11.0f}, {1, 1.0f}, {2, 1.01f} };
		REQUIRE_NOTHROW(set.size() == 5);
		REQUIRE_NOTHROW(!set.empty());
		REQUIRE_NOTHROW(std::distance(set.begin(), set.end()) == 5);
		REQUIRE_NOTHROW(++++++++++set.begin() == set.end());

		auto range = set.findSecondary(10);
		REQUIRE_NOTHROW(range.first == range.second);

		range = set.findSecondaryInRange(1.0f, 11.0f);
		REQUIRE_NOTHROW(range.first != range.second);
		REQUIRE_NOTHROW(std::distance(range.first, range.second) == 3);
		REQUIRE_NOTHROW((*range.first)->floatValue == 1.0f);
		++range.first;
		REQUIRE_NOTHROW((*range.first)->floatValue == 1.0f);
		++range.first;
		REQUIRE_NOTHROW((*range.first)->floatValue == 1.01f);
		REQUIRE_NOTHROW((*range.first)->intValue == 2);
		++range.first;
		REQUIRE_NOTHROW(range.first == range.second);

		range = set.findSecondaryInRange(1.0f, 1.02f);
		REQUIRE_NOTHROW(range.first != range.second);
		REQUIRE_NOTHROW(std::distance(range.first, range.second) == 2);
		REQUIRE_NOTHROW((*range.first)->floatValue == 1.0f && (*range.second)->intValue == 0);
		++range.first;
		REQUIRE_NOTHROW((*range.first)->floatValue == 1.0f && (*range.second)->intValue == 1);
		++range.first;
		REQUIRE_NOTHROW(range.first == range.second);

		range = set.findSecondaryInRange(1.0f, 1.0000f);
		REQUIRE_NOTHROW(range.first == range.second);
	}
	catch (...) {
		FAIL();
	}
}

TEST_CASE("Iterators test", "[multiindex]") {
	struct S {
		int intValue;
		float floatValue;
	};

	try {
		MultiIndexSet<S, &S::intValue, &S::floatValue> set{ {100, 111.0f}, {0, 1.0f}, {10, 11.0f} };

		SECTION("Primary")
		{
			REQUIRE_NOTHROW((*set.findPrimary(0)).intValue == 0 && (*set.findPrimary(0)).floatValue == 1.0f);
			REQUIRE_NOTHROW(set.findPrimary(0)->intValue == 0 && set.findPrimary(0)->floatValue == 1.0f);
		}

		SECTION("Secondary") {
			auto range = set.findSecondaryInRange(1.0f, 11.0f);
			REQUIRE_NOTHROW((*range.first)->floatValue == 1.0f && (*range.first)->intValue == 0);
			//REQUIRE_NOTHROW((range.first.operator->())->floatValue == 1.0f);
		}
	}
	catch (...) {
		FAIL();
	}
}