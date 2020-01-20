#pragma once

#include "catch2/catch.hpp"
#include "container/multi_index.hpp"

TEST_CASE("Construction", "[multiindex]") {
	struct S {
		int intValue;
		float floatValue;
	};

	MultiIndexSet<S, &S::intValue, &S::floatValue> set;
}
