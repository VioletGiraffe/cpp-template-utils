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
		REQUIRE_NOTHROW(set.findPrimary(0) != set.end());
		REQUIRE_NOTHROW(set.findPrimary(-7) == set.end());
		REQUIRE_NOTHROW(set.findPrimary(10) != set.end());
		REQUIRE_NOTHROW(set.begin() != set.end());

		auto range = set.findSecondary(10);
		REQUIRE_NOTHROW(range.first == range.second);
		range = set.findSecondaryInRange(100.0f, 500.0f);
		REQUIRE_NOTHROW(range.first == range.second);
		REQUIRE_NOTHROW(std::distance(range.first, range.second) == 0);
	} catch(...) {
		FAIL();
	}
}


