#pragma once

#include "../3rdparty/catch2/catch.hpp"
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
		REQUIRE(set.debugCheckSecondaryIndex());
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
		REQUIRE(set.debugCheckSecondaryIndex());
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
		SECTION("Empty - default constructor 1")
		{
			MultiIndexSet<S, &S::intValue, &S::floatValue> set;
			REQUIRE_NOTHROW(set.size() == 0);
			REQUIRE_NOTHROW(set.empty());
			REQUIRE_NOTHROW(std::distance(set.begin(), set.end()) == 0);
			REQUIRE_NOTHROW(set.begin() == set.end());
			REQUIRE(set.debugCheckSecondaryIndex());
		}

		SECTION("Empty - default constructor 2")
		{
			MultiIndexSet<S, &S::intValue, &S::floatValue> set{};
			REQUIRE_NOTHROW(set.size() == 0);
			REQUIRE_NOTHROW(set.empty());
			REQUIRE_NOTHROW(std::distance(set.begin(), set.end()) == 0);
			REQUIRE_NOTHROW(set.begin() == set.end());
			REQUIRE(set.debugCheckSecondaryIndex());
		}

		SECTION("Empty init list")
		{
			MultiIndexSet<S, &S::intValue, &S::floatValue> set{ std::initializer_list<S>{} };
			REQUIRE_NOTHROW(set.size() == 0);
			REQUIRE_NOTHROW(set.empty());
			REQUIRE_NOTHROW(std::distance(set.begin(), set.end()) == 0);
			REQUIRE_NOTHROW(set.begin() == set.end());
			REQUIRE(set.debugCheckSecondaryIndex());
		}

		SECTION("Initializer list")
		{
			MultiIndexSet<S, &S::intValue, &S::floatValue> set{ {100, 111.0f}, {0, 1.0f}, {10, 11.0f} };
			REQUIRE_NOTHROW(set.size() == 3);
			REQUIRE_NOTHROW(!set.empty());
			REQUIRE_NOTHROW(std::distance(set.begin(), set.end()) == 3);
			REQUIRE_NOTHROW(++++++set.begin() == set.end());

			REQUIRE_NOTHROW(set.begin()->floatValue == 1.0f && set.begin()->intValue == 0);
			REQUIRE_NOTHROW(((++set.begin())->floatValue == 11.0f) && ((++set.begin())->intValue == 10));
			REQUIRE_NOTHROW(((++++set.begin())->floatValue == 111.0f) && ((++++set.begin())->intValue == 100));
			REQUIRE_NOTHROW(((--set.end())->floatValue == 111.0f) && ((--set.end())->intValue == 100));
			REQUIRE(set.debugCheckSecondaryIndex());
		}
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
		REQUIRE(set.debugCheckSecondaryIndex());

		SECTION("By primary index")
		{
			REQUIRE_NOTHROW(set.findPrimary(0) != set.end());
			REQUIRE_NOTHROW(set.findPrimary(0) == set.begin());
			REQUIRE_NOTHROW(set.findPrimary(10) == ++set.begin());
			REQUIRE_NOTHROW(set.findPrimary(100) == ++++set.begin());
			REQUIRE_NOTHROW(set.findPrimary(-7) == set.end());
			REQUIRE_NOTHROW(set.findPrimary(10) != set.end());
			REQUIRE_NOTHROW(set.findPrimary(10.0f) == ++set.begin()); // Transaprent comparison test, "conversion from 'float' to 'const int'" warning intentional
			REQUIRE(set.debugCheckSecondaryIndex());
		}

		SECTION("By secondary index") {
			auto range = set.findSecondary(10);
			REQUIRE_NOTHROW(range.first == range.second);
			range = set.findSecondaryInRange(100.0f, 500.0f);
			REQUIRE_NOTHROW(range.first != range.second);
			REQUIRE_NOTHROW(std::distance(range.first, range.second) == 1);
			REQUIRE_NOTHROW((*range.first)->floatValue == 111.0f);
			REQUIRE(set.debugCheckSecondaryIndex());

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
			REQUIRE(set.debugCheckSecondaryIndex());

			range = set.findSecondaryInRange(1.0f, 11.01f);
			REQUIRE_NOTHROW(range.first != range.second);
			REQUIRE_NOTHROW(std::distance(range.first, range.second) == 2);
			REQUIRE_NOTHROW((*range.first)->floatValue == 1.0f);
			REQUIRE_NOTHROW((*++range.first)->floatValue == 11.0f);
			REQUIRE(set.debugCheckSecondaryIndex());
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
		REQUIRE(set.debugCheckSecondaryIndex());

		auto range = set.findSecondary(10);
		REQUIRE_NOTHROW(range.first == range.second);
		REQUIRE(set.debugCheckSecondaryIndex());

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
		REQUIRE(set.debugCheckSecondaryIndex());

		range = set.findSecondaryInRange(1.0f, 1.02f);
		REQUIRE_NOTHROW(range.first != range.second);
		REQUIRE_NOTHROW(std::distance(range.first, range.second) == 2);
		REQUIRE_NOTHROW((*range.first)->floatValue == 1.0f && (*range.second)->intValue == 0);
		++range.first;
		REQUIRE_NOTHROW((*range.first)->floatValue == 1.0f && (*range.second)->intValue == 1);
		++range.first;
		REQUIRE_NOTHROW(range.first == range.second);
		REQUIRE(set.debugCheckSecondaryIndex());

		range = set.findSecondaryInRange(1.0f, 1.0000f);
		REQUIRE_NOTHROW(range.first == range.second);
		REQUIRE(set.debugCheckSecondaryIndex());
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
			REQUIRE(set.debugCheckSecondaryIndex());
		}

		SECTION("Secondary") {
			auto range = set.findSecondaryInRange(1.0f, 11.0f);
			REQUIRE_NOTHROW((*range.first)->floatValue == 1.0f && (*range.first)->intValue == 0);
			REQUIRE(set.debugCheckSecondaryIndex());
			//REQUIRE_NOTHROW((range.first.operator->())->floatValue == 1.0f);
		}
	}
	catch (...) {
		FAIL();
	}
}

TEST_CASE("emplace", "[multiindex]") {
	try {
		SECTION("Empty set, emplacing S&&")
		{
			struct S {
				S(int i, float f) : intValue{ i }, floatValue{ f } {}

				int intValue = 0;
				float floatValue = 0.0f;
			};

			MultiIndexSet<S, &S::intValue, &S::floatValue> set{};
			REQUIRE(set.debugCheckSecondaryIndex());

			auto it = set.emplace(S{ 1, 2.0f });
			REQUIRE(it.second == true);
			REQUIRE(it.first != set.end());
			REQUIRE(!set.empty());
			REQUIRE(set.size() == 1);
			REQUIRE(it.first->intValue == 1);
			REQUIRE(it.first->floatValue == 2.0f);
			REQUIRE(set.debugCheckSecondaryIndex());

			it = set.emplace(S{ 2, 2.0f });
			REQUIRE(it.second == true);
			REQUIRE(it.first != set.end());
			REQUIRE(!set.empty());
			REQUIRE(set.size() == 2);
			REQUIRE(it.first->intValue == 2);
			REQUIRE(it.first->floatValue == 2.0f);
			REQUIRE(set.debugCheckSecondaryIndex());

			it = set.emplace(S{ 3, 1.0f });
			REQUIRE(it.second == true);
			REQUIRE(it.first != set.end());
			REQUIRE(!set.empty());
			REQUIRE(set.size() == 3);
			REQUIRE(it.first->intValue == 3);
			REQUIRE(it.first->floatValue == 1.0f);
			REQUIRE(std::distance(it.first, set.end()) == 1);
			REQUIRE(set.debugCheckSecondaryIndex());
		}

		SECTION("Empty set, in-place item construction")
		{
			struct S {
				S(int i, float f) : intValue{ i }, floatValue{ f } {}

				int intValue = 0;
				float floatValue = 0.0f;
			};

			MultiIndexSet<S, &S::intValue, &S::floatValue> set{};
			REQUIRE(set.debugCheckSecondaryIndex());

			auto it = set.emplace(1, 2.0f);
			REQUIRE(it.second == true);
			REQUIRE(it.first != set.end());
			REQUIRE(!set.empty());
			REQUIRE(set.size() == 1);
			REQUIRE(it.first->intValue == 1);
			REQUIRE(it.first->floatValue == 2.0f);
			REQUIRE(set.debugCheckSecondaryIndex());

			it = set.emplace(2, 2.0f);
			REQUIRE(it.second == true);
			REQUIRE(it.first != set.end());
			REQUIRE(!set.empty());
			REQUIRE(set.size() == 2);
			REQUIRE(it.first->intValue == 2);
			REQUIRE(it.first->floatValue == 2.0f);
			REQUIRE(set.debugCheckSecondaryIndex());

			it = set.emplace(3, 1.0f);
			REQUIRE(it.second == true);
			REQUIRE(it.first != set.end());
			REQUIRE(!set.empty());
			REQUIRE(set.size() == 3);
			REQUIRE(it.first->intValue == 3);
			REQUIRE(it.first->floatValue == 1.0f);
			REQUIRE(std::distance(it.first, set.end()) == 1);
			REQUIRE(set.debugCheckSecondaryIndex());
		}

		SECTION("Pre-filled set")
		{
			struct S {
				int intValue;
				float floatValue;
			};

			MultiIndexSet<S, &S::intValue, &S::floatValue> set{ {100, 111.0f}, {0, 1.0f}, {10, 11.0f} };
			const auto initialSize = set.size();
			REQUIRE(initialSize > 0);
			REQUIRE(set.debugCheckSecondaryIndex());

			auto it = set.emplace(S{ 1, 2.0f });
			REQUIRE(it.second == true);
			REQUIRE(it.first != set.end());
			REQUIRE(!set.empty());
			REQUIRE(set.size() == 1 + initialSize);
			REQUIRE(it.first->intValue == 1);
			REQUIRE(it.first->floatValue == 2.0f);
			REQUIRE(set.debugCheckSecondaryIndex());

			it = set.emplace(S{ 2, 2.0f });
			REQUIRE(it.second == true);
			REQUIRE(it.first != set.end());
			REQUIRE(!set.empty());
			REQUIRE(set.size() == 2 + initialSize);
			REQUIRE(it.first->intValue == 2);
			REQUIRE(it.first->floatValue == 2.0f);
			REQUIRE(set.debugCheckSecondaryIndex());

			it = set.emplace(S{ 3, 1.0f });
			REQUIRE(it.second == true);
			REQUIRE(it.first != set.end());
			REQUIRE(!set.empty());
			REQUIRE(set.size() == 3 + initialSize);
			REQUIRE(it.first->intValue == 3);
			REQUIRE(it.first->floatValue == 1.0f);
			REQUIRE(std::distance(it.first, set.end()) == 3);
			REQUIRE(set.debugCheckSecondaryIndex());
		}

		SECTION("Emplacing a duplicate primary key")
		{
			struct S {
				std::string s;
				float floatValue;
			};

			MultiIndexSet<S, &S::s, &S::floatValue> set{ {"100", 111.0f}, {"0", 1.0f}, {"10", 11.0f} };
			const auto initialSize = set.size();
			REQUIRE(initialSize > 0);
			REQUIRE(set.debugCheckSecondaryIndex());

			auto it = set.emplace(S{ "10", 2.0f });
			REQUIRE(it.second == false);
			REQUIRE(it.first != set.end());
			REQUIRE(!set.empty());
			REQUIRE(set.size() == initialSize);
			REQUIRE(it.first->s == "10");
			REQUIRE(it.first->floatValue == 11.0f);
			REQUIRE(set.debugCheckSecondaryIndex());

			it = set.emplace(S{ "2", 2.0f });
			REQUIRE(it.second == true);
			REQUIRE(it.first != set.end());
			REQUIRE(!set.empty());
			REQUIRE(set.size() == 1 + initialSize);
			REQUIRE(it.first->s == "2");
			REQUIRE(it.first->floatValue == 2.0f);
			REQUIRE(set.debugCheckSecondaryIndex());
		}
	}
	catch (...) {
		FAIL();
	}
}

TEST_CASE("erase", "[multiindex]") {
	try {
		SECTION("Empty set")
		{
			struct S {
				int intValue = 0;
				float floatValue = 0.0f;

				bool operator==(const S& other) const noexcept {
					return intValue == other.intValue && floatValue == other.floatValue;
				}
			};

			MultiIndexSet<S, &S::intValue, &S::floatValue> set{};

			REQUIRE(set.erase(0) == 0);
			REQUIRE(set.erase(S{ 1, 2.0f }) == 0);
			REQUIRE(set.empty());
			REQUIRE(set.debugCheckSecondaryIndex());
		}

		SECTION("Pre-filled set, erasing by iterator")
		{
			struct S {
				int intValue;
				float floatValue;
			};

			MultiIndexSet<S, &S::intValue, &S::floatValue> set{ {100, 111.0f}, {0, 1.0f}, {10, 11.0f} };
			const auto initialSize = set.size();
			REQUIRE(initialSize > 0);
			REQUIRE(set.debugCheckSecondaryIndex());

			REQUIRE(set.findPrimary(0) != set.end());
			set.erase(set.begin());
			REQUIRE(set.size() == initialSize - 1);
			REQUIRE(set.findPrimary(0) == set.end());
			REQUIRE(set.debugCheckSecondaryIndex());
		}

		SECTION("Pre-filled set, erasing by primary key")
		{
			struct S {
				int intValue;
				float floatValue;
			};

			MultiIndexSet<S, &S::intValue, &S::floatValue> set{ {100, 111.0f}, {0, 1.0f}, {10, 11.0f} };
			const auto initialSize = set.size();
			REQUIRE(initialSize > 0);
			REQUIRE(set.debugCheckSecondaryIndex());

			REQUIRE(set.findPrimary(0) != set.end());
			REQUIRE(set.erase(0) == 1);
			REQUIRE(set.size() == initialSize - 1);
			REQUIRE(set.findPrimary(0) == set.end());
			REQUIRE(set.debugCheckSecondaryIndex());

			REQUIRE(set.erase(0) == 0);
			REQUIRE(set.size() == initialSize - 1);
			REQUIRE(set.findPrimary(0) == set.end());
			REQUIRE(set.debugCheckSecondaryIndex());
		}

		SECTION("Pre-filled set, erasing by item value")
		{
			struct S {
				int intValue;
				float floatValue;

				bool operator==(const S& other) const noexcept {
					return intValue == other.intValue && floatValue == other.floatValue;
				}
			};

			MultiIndexSet<S, &S::intValue, &S::floatValue> set{ {100, 111.0f}, {0, 1.0f}, {10, 11.0f} };
			const auto initialSize = set.size();
			REQUIRE(initialSize > 0);
			REQUIRE(set.debugCheckSecondaryIndex());

			REQUIRE(set.findPrimary(10) != set.end());
			REQUIRE(set.erase(S{10, 0.0f}) == 0);
			REQUIRE(set.size() == initialSize);
			REQUIRE(set.findPrimary(10) != set.end());
			REQUIRE(set.debugCheckSecondaryIndex());

			REQUIRE(set.erase(S{ 10, 11.0f }) == 1);
			REQUIRE(set.size() == initialSize - 1);
			REQUIRE(set.findPrimary(10) == set.end());

			REQUIRE(set.findPrimary(100) != set.end());
			REQUIRE(set.findPrimary(0) != set.end());

			REQUIRE(set.debugCheckSecondaryIndex());
		}
	}
	catch (...) {
		FAIL();
	}
}