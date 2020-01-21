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
	} catch(...) {
		FAIL();
	}
}


