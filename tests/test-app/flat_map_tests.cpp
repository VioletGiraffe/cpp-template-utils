#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include "3rdparty/catch2/catch.hpp"
RESTORE_COMPILER_WARNINGS

#include "container/flat_map.hpp"

#include <algorithm>
#include <compare>
#include <concepts>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

	struct counted_key
	{
		int value;
		static inline int equality_calls = 0;

		friend bool operator==(const counted_key& left, const counted_key& right)
		{
			++equality_calls;
			return left.value == right.value;
		}
	};

	struct counted_less
	{
		static inline int calls = 0;

		bool operator()(const counted_key& left, const counted_key& right) const
		{
			++calls;
			return left.value < right.value;
		}
	};

	struct ordered_only_key
	{
		int value;
	};

	struct ordered_only_less
	{
		bool operator()(const ordered_only_key& left, const ordered_only_key& right) const { return left.value < right.value; }
	};

	struct directional_less
	{
		bool descending = false;

		bool operator()(int left, int right) const { return descending ? left > right : left < right; }
	};

	// tag is invisible to tagged_less, so equivalent values stay distinguishable
	struct tagged_value
	{
		int key;
		int tag;
	};

	struct tagged_less
	{
		bool operator()(const tagged_value& left, const tagged_value& right) const { return left.key < right.key; }
	};

	struct move_only_value
	{
		explicit move_only_value(int value): value(value) {}
		move_only_value() = delete;
		move_only_value(const move_only_value&) = delete;
		move_only_value& operator=(const move_only_value&) = delete;

		move_only_value(move_only_value&& other) noexcept: value(other.value) { other.value = -1; }
		move_only_value& operator=(move_only_value&& other) noexcept
		{
			value = other.value;
			other.value = -1;
			return *this;
		}

		friend bool operator<(const move_only_value& left, const move_only_value& right) { return left.value < right.value; }

		int value;
	};

	struct throwing_value
	{
		struct construction_failed {};

		static constexpr int poison = -1;

		explicit throwing_value(int value): value(value)
		{
			if (value == poison)
				throw construction_failed{};
		}

		int value;
	};

	struct throwing_less
	{
		bool operator()(const throwing_value& left, const throwing_value& right) const { return left.value < right.value; }
	};

	struct comparison_failed {};

	// The comparator is the throw site reachable from inside end_batch(), so arming it fails the merge itself
	struct armable_less
	{
		static inline int comparisons_until_throw = -1; // Negative: disarmed

		bool operator()(int left, int right) const
		{
			if (comparisons_until_throw >= 0 && comparisons_until_throw-- == 0)
				throw comparison_failed{};
			return left < right;
		}
	};

} // namespace

TEST_CASE("flat_map supports pair-like random-access iteration", "[flat-map]")
{
	flat_map<int, std::string> map{ { 3, "three" }, { 1, "one" }, { 2, "two" } };

	static_assert(std::random_access_iterator<decltype(map.begin())>);
	CHECK(map.begin()->first == 1);
	CHECK(map.begin()->second == "one");
	CHECK(map.begin().key() == 1);
	CHECK(map.begin().value() == "one");

	map.begin()->second = "changed";
	CHECK(map.at(1) == "changed");
	auto [first_key, first_value] = *map.begin();
	first_value = "changed again";
	CHECK(first_key == 1);
	CHECK(map.at(1) == "changed again");

	const auto& const_map = map;
	CHECK((const_map.end() - const_map.begin()) == 3);
	CHECK((const_map.begin() + 2)->first == 3);
	CHECK(std::find(const_map.begin(), const_map.end(), std::pair<int, std::string>{ 2, "two" }) == const_map.begin() + 1);

	const std::vector<std::pair<int, std::string>> copied(const_map.begin(), const_map.end());
	CHECK((copied == std::vector<std::pair<int, std::string>>{ { 1, "changed again" }, { 2, "two" }, { 3, "three" } }));
}

TEST_CASE("flat containers compare keys and mapped values", "[flat-map][flat-set]")
{
	const flat_map<int, std::string> map{ { 2, "two" }, { 1, "one" } };
	CHECK((map == flat_map<int, std::string>{ { 1, "one" }, { 2, "two" } }));
	CHECK_FALSE((map == flat_map<int, std::string>{ { 1, "one" }, { 2, "changed" } }));
	CHECK_FALSE((map == flat_map<int, std::string>{ { 1, "one" } }));

	const flat_set<int> set{ 2, 1 };
	CHECK((set == flat_set<int>{ 1, 2 }));
	CHECK_FALSE((set == flat_set<int>{ 1, 3 }));
}

TEST_CASE("flat containers use equality after lower bound for duplicate detection", "[flat-map][flat-set]")
{
	flat_map<counted_key, int, counted_less> map;
	map.try_emplace(counted_key{ 1 }, 10);
	counted_key::equality_calls = 0;
	counted_less::calls = 0;
	CHECK_FALSE(map.try_emplace(counted_key{ 1 }, 20).second);
	CHECK(counted_key::equality_calls == 1);
	CHECK(counted_less::calls == 1);

	flat_set<counted_key, counted_less> set;
	set.insert(counted_key{ 1 });
	counted_key::equality_calls = 0;
	counted_less::calls = 0;
	CHECK_FALSE(set.insert(counted_key{ 1 }).second);
	CHECK(counted_key::equality_calls == 1);
	CHECK(counted_less::calls == 1);
}

TEST_CASE("flat_map supports heterogeneous lookup", "[flat-map]")
{
	flat_map<std::string, int> map{ { "one", 1 }, { "two", 2 } };
	const auto position = map.find(std::string_view("two"));

	CHECK(position != map.end());
	CHECK(position->second == 2);
	CHECK(map.contains(std::string_view("one")));
}

TEST_CASE("flat_map merges sorted bulk insertion", "[flat-map]")
{
	flat_map<int, std::string> map{ { 2, "existing" }, { 4, "four" } };
	const std::vector<std::pair<int, std::string>> incoming{
		{ 1, "one" }, { 2, "replacement" }, { 3, "first three" }, { 3, "second three" }, { 5, "five" }
	};

	map.insert_sorted(incoming.begin(), incoming.end());

	const std::vector<std::pair<int, std::string>> expected{
		{ 1, "one" }, { 2, "existing" }, { 3, "first three" }, { 4, "four" }, { 5, "five" }
	};
	CHECK(std::equal(map.begin(), map.end(), expected.begin(), expected.end()));
}

TEST_CASE("flat_map appends strictly ordered unique values without consuming rejected values", "[flat-map]")
{
	flat_map<int, move_only_value> map;
	map.reserve(3);
	CHECK(map.append_sorted_unique(1, move_only_value(10)));
	CHECK(map.append_sorted_unique(3, move_only_value(30)));

	move_only_value duplicate(300);
	CHECK_FALSE(map.append_sorted_unique(3, std::move(duplicate)));
	CHECK(duplicate.value == 300);
	move_only_value rejected(20);
	CHECK_FALSE(map.append_sorted_unique(2, std::move(rejected)));
	CHECK(rejected.value == 20);
	CHECK(map.size() == 2);
	CHECK(map.at(1).value == 10);
	CHECK(map.at(3).value == 30);
}

TEST_CASE("flat_map bulk operations handle empty and non-overlapping ranges", "[flat-map]")
{
	flat_map<int, int> map;
	const std::vector<std::pair<int, int>> empty;
	map.insert_sorted(empty.begin(), empty.end());
	map.begin_batch();
	map.end_batch();
	CHECK(map.empty());

	const std::vector<std::pair<int, int>> initial{ { 10, 100 }, { 20, 200 } };
	map.insert_sorted(initial.begin(), initial.end());
	const std::vector<std::pair<int, int>> before{ { 1, 10 }, { 2, 20 } };
	map.insert_sorted(before.begin(), before.end());
	const std::vector<std::pair<int, int>> after{ { 30, 300 }, { 40, 400 } };
	map.insert_sorted(after.begin(), after.end());

	const std::vector<std::pair<int, int>> expected{ { 1, 10 }, { 2, 20 }, { 10, 100 }, { 20, 200 }, { 30, 300 }, { 40, 400 } };
	CHECK(std::equal(map.begin(), map.end(), expected.begin(), expected.end()));
}

TEST_CASE("flat_map batch insertion sorts only the appended tail before merging", "[flat-map]")
{
	flat_map<int, std::string> map{ { 2, "existing" }, { 5, "five" } };
	map.begin_batch();
	map.append_unsorted(4, "first four");
	map.append_unsorted(1, "one");
	map.append_unsorted(4, "second four");
	map.append_unsorted(2, "replacement");
	map.end_batch();

	const std::vector<std::pair<int, std::string>> expected{
		{ 1, "one" }, { 2, "existing" }, { 4, "first four" }, { 5, "five" }
	};
	CHECK(std::equal(map.begin(), map.end(), expected.begin(), expected.end()));
}

TEST_CASE("flat_map supports vector bool mapped references", "[flat-map]")
{
	flat_map<int, bool> map;
	map[1] = true;
	map.insert_or_assign(2, false);
	map.find(2)->second = true;

	CHECK(map.at(1));
	CHECK(map.at(2));
}

TEST_CASE("flat_map supports move-only non-default-constructible mapped values", "[flat-map]")
{
	flat_map<int, move_only_value> map;
	CHECK(map.try_emplace(2, 20).second);

	move_only_value unused_duplicate(200);
	CHECK_FALSE(map.try_emplace(2, std::move(unused_duplicate)).second);
	CHECK(unused_duplicate.value == 200);

	map.begin_batch();
	map.append_unsorted(1, move_only_value(10));
	map.append_unsorted(2, move_only_value(200));
	map.end_batch();
	CHECK(map.at(1).value == 10);
	CHECK(map.at(2).value == 20);

	std::vector<std::pair<int, move_only_value>> incoming;
	incoming.emplace_back(0, move_only_value(0));
	incoming.emplace_back(3, move_only_value(30));
	map.insert_sorted(std::make_move_iterator(incoming.begin()), std::make_move_iterator(incoming.end()));
	CHECK(map.at(0).value == 0);
	CHECK(map.at(3).value == 30);

	move_only_value replacement(22);
	CHECK_FALSE(map.insert_or_assign(2, std::move(replacement)).second);
	CHECK(replacement.value == -1);
	CHECK(map.at(2).value == 22);
}

TEST_CASE("flat_map lookup and erasure cover boundaries", "[flat-map]")
{
	flat_map<int, int> map{ { 1, 10 }, { 3, 30 }, { 5, 50 }, { 7, 70 } };
	CHECK(map.lower_bound(0) == map.begin());
	CHECK(map.lower_bound(2)->first == 3);
	CHECK(map.upper_bound(3)->first == 5);
	CHECK(map.lower_bound(8) == map.end());
	CHECK(map.count(5) == 1);
	CHECK(map.count(6) == 0);
	CHECK_THROWS_AS(map.at(6), std::out_of_range);

	auto next = map.erase(map.find(1));
	CHECK(next->first == 3);
	next = map.erase(map.find(7));
	CHECK(next == map.end());
	CHECK(map.erase(100) == 0);
	CHECK(map.erase(3) == 1);
	CHECK(map.size() == 1);
	CHECK(map.begin()->first == 5);

	map.insert_or_assign(3, 30);
	map.insert_or_assign(7, 70);
	next = map.erase(map.find(3), map.find(7));
	CHECK(next->first == 7);
	CHECK(map.size() == 1);
}

TEST_CASE("flat_map iterator interoperates across constness and standard copying", "[flat-map]")
{
	flat_map<int, int> map{ { 1, 10 }, { 2, 20 }, { 3, 30 } };
	static_assert(std::convertible_to<flat_map<int, int>::iterator, flat_map<int, int>::const_iterator>);

	auto position = map.begin();
	auto previous = position++;
	CHECK(previous->first == 1);
	CHECK(position->first == 2);
	CHECK((position--)->first == 2);
	CHECK(position->first == 1);
	CHECK(position[2].second == 30);

	flat_map<int, int>::const_iterator const_position = position;
	CHECK(const_position == position);
	CHECK((map.cend() - position) == 3);

	std::vector<std::pair<int, int>> copied;
	std::copy(map.begin(), map.end(), std::back_inserter(copied));
	CHECK((copied == std::vector<std::pair<int, int>>{ { 1, 10 }, { 2, 20 }, { 3, 30 } }));
}

TEST_CASE("flat containers fall back to comparator equivalence without operator equality", "[flat-map][flat-set]")
{
	flat_map<ordered_only_key, int, ordered_only_less> map;
	CHECK(map.try_emplace(ordered_only_key{ 1 }, 10).second);
	CHECK_FALSE(map.try_emplace(ordered_only_key{ 1 }, 20).second);
	CHECK(map.at(ordered_only_key{ 1 }) == 10);

	flat_set<ordered_only_key, ordered_only_less> set;
	CHECK(set.insert(ordered_only_key{ 1 }).second);
	CHECK_FALSE(set.insert(ordered_only_key{ 1 }).second);
}

TEST_CASE("flat containers preserve stateful reverse comparators", "[flat-map][flat-set]")
{
	flat_map<int, int, directional_less> map(directional_less{ true });
	map.try_emplace(3, 30);
	map.try_emplace(1, 10);
	const std::vector<std::pair<int, int>> incoming{ { 4, 40 }, { 2, 20 }, { 0, 0 } };
	map.insert_sorted(incoming.begin(), incoming.end());
	CHECK(map.key_comp().descending);

	std::vector<int> keys;
	for (const auto entry : map)
		keys.push_back(entry.first);
	CHECK((keys == std::vector<int>{ 4, 3, 2, 1, 0 }));

	auto copied = map;
	auto moved = std::move(copied);
	CHECK(moved.key_comp().descending);
	CHECK(moved.begin()->first == 4);

	flat_set<int, directional_less> set(directional_less{ true });
	set.insert(1);
	set.insert(3);
	set.insert(2);
	CHECK(set.key_comp().descending);
	const std::vector<int> expected_set{ 3, 2, 1 };
	CHECK(std::equal(set.begin(), set.end(), expected_set.begin(), expected_set.end()));
}

TEST_CASE("flat containers match standard ordered containers through mixed operations", "[flat-map][flat-set]")
{
	flat_map<int, int> map;
	std::map<int, int> expected_map;
	flat_set<int> set;
	std::set<int> expected_set;

	const auto check_map = [&] {
		REQUIRE(map.size() == expected_map.size());
		auto actual = map.begin();
		for (const auto& [expected_key, expected_value] : expected_map) {
			REQUIRE(actual != map.end());
			CHECK(actual->first == expected_key);
			CHECK(actual->second == expected_value);
			++actual;
		}
		CHECK(actual == map.end());
	};
	const auto check_set = [&] {
		REQUIRE(set.size() == expected_set.size());
		CHECK(std::equal(set.begin(), set.end(), expected_set.begin(), expected_set.end()));
	};

	for (const auto [key, value] : std::vector<std::pair<int, int>>{ { 5, 50 }, { 1, 10 }, { 3, 30 }, { 5, 500 } }) {
		map.try_emplace(key, value);
		expected_map.try_emplace(key, value);
		set.insert(key);
		expected_set.insert(key);
		check_map();
		check_set();
	}

	map.insert_or_assign(3, 33);
	expected_map.insert_or_assign(3, 33);
	check_map();

	CHECK(map.erase(1) == expected_map.erase(1));
	CHECK(set.erase(1) == expected_set.erase(1));
	check_map();
	check_set();

	const std::vector<std::pair<int, int>> sorted_map_entries{ { 0, 0 }, { 3, 300 }, { 4, 40 }, { 8, 80 }, { 8, 81 } };
	map.insert_sorted(sorted_map_entries.begin(), sorted_map_entries.end());
	for (const auto& entry : sorted_map_entries)
		expected_map.insert(entry);
	const std::vector<int> sorted_set_entries{ 0, 3, 4, 8, 8 };
	set.insert_sorted(sorted_set_entries.begin(), sorted_set_entries.end());
	expected_set.insert(sorted_set_entries.begin(), sorted_set_entries.end());
	check_map();
	check_set();

	const std::vector<std::pair<int, int>> batch_entries{ { 7, 70 }, { 2, 20 }, { 7, 700 }, { 5, 500 } };
	map.begin_batch();
	set.begin_batch();
	for (const auto& [key, value] : batch_entries) {
		map.append_unsorted(key, value);
		set.append_unsorted(key);
		expected_map.insert({ key, value });
		expected_set.insert(key);
	}
	map.end_batch();
	set.end_batch();
	check_map();
	check_set();

	CHECK(map.erase(100) == expected_map.erase(100));
	CHECK(set.erase(100) == expected_set.erase(100));
	check_map();
	check_set();

	map.erase(map.lower_bound(3), map.upper_bound(7));
	expected_map.erase(expected_map.lower_bound(3), expected_map.upper_bound(7));
	set.erase(set.lower_bound(3), set.upper_bound(7));
	expected_set.erase(expected_set.lower_bound(3), expected_set.upper_bound(7));
	check_map();
	check_set();
}

TEST_CASE("flat containers match standard ordered containers through an initial batch", "[flat-map][flat-set]")
{
	// A batch opened on an empty container is sorted and deduplicated in place, never merged against a prefix
	const std::vector<std::pair<int, int>> entries{
		{ 5, 50 }, { 1, 10 }, { 5, 500 }, { 3, 30 }, { 1, 100 }, { 5, 5000 }, { 2, 20 }
	};

	flat_map<int, int> map;
	std::map<int, int> expected_map;
	flat_set<int> set;
	std::set<int> expected_set;

	map.begin_batch();
	set.begin_batch();
	for (const auto& [key, value] : entries) {
		map.append_unsorted(key, value);
		set.append_unsorted(key);
		expected_map.insert({ key, value });
		expected_set.insert(key);
	}
	map.end_batch();
	set.end_batch();

	REQUIRE(map.size() == expected_map.size());
	CHECK(std::equal(map.begin(), map.end(), expected_map.begin(), expected_map.end()));
	REQUIRE(set.size() == expected_set.size());
	CHECK(std::equal(set.begin(), set.end(), expected_set.begin(), expected_set.end()));
}

TEST_CASE("flat_map survives a mapped value that throws while being inserted", "[flat-map]")
{
	flat_map<int, throwing_value> map;
	map.try_emplace(1, 10);
	map.try_emplace(3, 30);

	SECTION("try_emplace")
	{
		CHECK_THROWS_AS(map.try_emplace(2, throwing_value::poison), throwing_value::construction_failed);
	}

	SECTION("append_sorted_unique")
	{
		CHECK_THROWS_AS(map.append_sorted_unique(9, throwing_value::poison), throwing_value::construction_failed);
	}

	SECTION("append_unsorted")
	{
		map.begin_batch();
		CHECK_THROWS_AS(map.append_unsorted(9, throwing_value::poison), throwing_value::construction_failed);
		// Checked before end_batch, which would run off the shorter value vector
		REQUIRE(map.size() == 2);
		map.end_batch();
	}

	// size() comes from the key vector, so a key left behind by the failed insertion is reported as an entry with no value
	REQUIRE(map.size() == 2);
	CHECK(map.at(1).value == 10);
	CHECK(map.at(3).value == 30);

	CHECK(map.try_emplace(2, 20).second);
	CHECK(map.size() == 3);
	CHECK(map.at(2).value == 20);
}

TEST_CASE("aborting a batch restores the state from before begin_batch", "[flat-map][flat-set]")
{
	flat_map<int, int> map{ { 1, 10 }, { 3, 30 } };
	flat_set<int> set{ 1, 3 };

	SECTION("appended entries are discarded")
	{
		map.begin_batch();
		map.append_unsorted(2, 20);
		map.append_unsorted(1, 100);
		map.abort_batch();

		set.begin_batch();
		set.append_unsorted(2);
		set.append_unsorted(1);
		set.abort_batch();
	}

	SECTION("an empty batch leaves everything in place")
	{
		map.begin_batch();
		map.abort_batch();

		set.begin_batch();
		set.abort_batch();
	}

	CHECK_FALSE(map.batch_open());
	CHECK_FALSE(set.batch_open());

	const std::vector<std::pair<int, int>> expected_map{ { 1, 10 }, { 3, 30 } };
	CHECK(std::equal(map.begin(), map.end(), expected_map.begin(), expected_map.end()));
	const std::vector<int> expected_set{ 1, 3 };
	CHECK(std::equal(set.begin(), set.end(), expected_set.begin(), expected_set.end()));

	// Ordered operations are valid again, and the containers take further entries normally
	CHECK(map.find(3) != map.end());
	CHECK(map.try_emplace(2, 20).second);
	CHECK(map.size() == 3);
	CHECK(set.insert(2).second);
	CHECK(set.size() == 3);
}

TEST_CASE("aborting a batch opened on an empty container leaves it empty", "[flat-map][flat-set]")
{
	flat_map<int, int> map;
	map.begin_batch();
	map.append_unsorted(1, 10);
	map.abort_batch();

	flat_set<int> set;
	set.begin_batch();
	set.append_unsorted(1);
	set.abort_batch();

	CHECK(map.empty());
	CHECK(set.empty());
	CHECK(map.begin() == map.end());
	CHECK(set.begin() == set.end());
}

TEST_CASE("abort_batch recovers a flat_map from a throwing append", "[flat-map]")
{
	flat_map<int, throwing_value> map;
	map.try_emplace(1, 10);
	map.try_emplace(3, 30);

	map.begin_batch();
	map.append_unsorted(2, 20);
	CHECK_THROWS_AS(map.append_unsorted(9, throwing_value::poison), throwing_value::construction_failed);
	map.abort_batch();

	REQUIRE(map.size() == 2);
	CHECK(map.at(1).value == 10);
	CHECK(map.at(3).value == 30);
	// Appended before the throw, and discarded with the rest of the batch
	CHECK(map.find(2) == map.end());
}

TEST_CASE("flat_map exposes keys and mapped values as parallel vectors", "[flat-map]")
{
	flat_map<int, std::string> map{ { 3, "three" }, { 1, "one" }, { 2, "two" } };

	CHECK((map.keys() == std::vector<int>{ 1, 2, 3 }));
	CHECK((map.values() == std::vector<std::string>{ "one", "two", "three" }));

	SECTION("a cleared map yields empty views")
	{
		map.clear();
		CHECK(map.keys().empty());
		CHECK(map.values().empty());
	}

	SECTION("insertion in the middle keeps the two aligned")
	{
		map.try_emplace(0, "zero");
		map.insert_or_assign(2, "TWO");
		CHECK((map.keys() == std::vector<int>{ 0, 1, 2, 3 }));
		CHECK((map.values() == std::vector<std::string>{ "zero", "one", "TWO", "three" }));
	}

	SECTION("erasure keeps the two aligned")
	{
		map.erase(1);
		CHECK((map.keys() == std::vector<int>{ 2, 3 }));
		CHECK((map.values() == std::vector<std::string>{ "two", "three" }));
	}

	SECTION("a merged batch shows up sorted and deduplicated")
	{
		map.begin_batch();
		map.append_unsorted(5, "five");
		map.append_unsorted(0, "zero");
		map.append_unsorted(2, "loses to the existing entry");
		map.end_batch();
		CHECK((map.keys() == std::vector<int>{ 0, 1, 2, 3, 5 }));
		CHECK((map.values() == std::vector<std::string>{ "zero", "one", "two", "three", "five" }));
	}

	SECTION("a value assigned through the map is visible in the view")
	{
		map.at(2) = "changed";
		CHECK(map.values()[1] == "changed");
	}

	SECTION("the views index in lockstep with iteration")
	{
		REQUIRE(map.keys().size() == map.size());
		REQUIRE(map.values().size() == map.size());

		auto index = flat_map<int, std::string>::size_type{ 0 };
		for (const auto entry : map) {
			CHECK(entry.first == map.keys()[index]);
			CHECK(entry.second == map.values()[index]);
			++index;
		}
	}

	SECTION("the key view is sorted by the map's comparator")
	{
		CHECK(std::is_sorted(map.keys().begin(), map.keys().end(), map.key_comp()));
	}
}

TEST_CASE("flat_map key and value views are read-only", "[flat-map]")
{
	flat_map<int, std::string> map{ { 1, "one" } };
	static_assert(std::is_same_v<decltype(map.keys()), const std::vector<int>&>);
	static_assert(std::is_same_v<decltype(map.values()), const std::vector<std::string>&>);

	// vector<bool> is not contiguous: the views must stay vector references, not spans
	flat_map<int, bool> flags{ { 2, true }, { 1, false } };
	CHECK((flags.keys() == std::vector<int>{ 1, 2 }));
	CHECK((flags.values() == std::vector<bool>{ false, true }));
}

TEST_CASE("flat_map iterates in reverse", "[flat-map]")
{
	flat_map<int, std::string> map{ { 3, "three" }, { 1, "one" }, { 2, "two" } };

	static_assert(std::random_access_iterator<decltype(map.rbegin())>);
	static_assert(std::random_access_iterator<decltype(map.crbegin())>);

	SECTION("entries come out in descending key order")
	{
		const std::vector<std::pair<int, std::string>> expected{ { 3, "three" }, { 2, "two" }, { 1, "one" } };
		CHECK(std::equal(map.rbegin(), map.rend(), expected.begin(), expected.end()));
	}

	SECTION("the proxy survives the arrow operator")
	{
		CHECK(map.rbegin()->first == 3);
		CHECK(map.rbegin()->second == "three");
		CHECK(map.rbegin().base() == map.end());
	}

	SECTION("a mapped value is writable through a reverse iterator")
	{
		map.rbegin()->second = "changed";
		CHECK(map.at(3) == "changed");
	}

	SECTION("the reverse range spans the whole map")
	{
		CHECK((map.rend() - map.rbegin()) == 3);
		CHECK(map.rbegin()[2].first == 1);
	}

	SECTION("const and non-const reverse iterators interoperate")
	{
		const auto& const_map = map;
		CHECK(map.crbegin() == const_map.rbegin());
		CHECK(map.crend() == const_map.rend());

		const flat_map<int, std::string>::const_reverse_iterator converted = map.rbegin();
		CHECK(converted == map.crbegin());
	}

	SECTION("an empty map has an empty reverse range")
	{
		map.clear();
		CHECK(map.rbegin() == map.rend());
		CHECK(map.crbegin() == map.crend());
	}
}

TEST_CASE("flat_map exposes the first and last entries", "[flat-map]")
{
	flat_map<int, std::string> map{ { 3, "three" }, { 1, "one" }, { 2, "two" } };

	CHECK(map.front().first == 1);
	CHECK(map.front().second == "one");
	CHECK(map.back().first == 3);
	CHECK(map.back().second == "three");

	SECTION("they follow insertion at either end")
	{
		map.try_emplace(0, "zero");
		map.try_emplace(9, "nine");
		CHECK(map.front().first == 0);
		CHECK(map.back().first == 9);
	}

	SECTION("a mapped value is writable through either")
	{
		map.front().second = "first";
		map.back().second = "last";
		CHECK(map.at(1) == "first");
		CHECK(map.at(3) == "last");
	}

	SECTION("a const map hands out const mapped references")
	{
		const auto& const_map = map;
		static_assert(std::is_same_v<decltype(const_map.front().second), const std::string&>);
		CHECK(const_map.front().second == "one");
		CHECK(const_map.back().second == "three");
	}

	SECTION("a single entry is both the first and the last")
	{
		const flat_map<int, std::string> single{ { 7, "seven" } };
		CHECK(single.front().first == 7);
		CHECK(single.back().first == 7);
	}
}

TEST_CASE("flat_map inserts an unsorted range", "[flat-map]")
{
	flat_map<int, std::string> map{ { 2, "existing" }, { 5, "five" } };

	SECTION("the range is sorted and merged, existing entries winning")
	{
		const std::vector<std::pair<int, std::string>> incoming{
			{ 4, "four" }, { 1, "one" }, { 2, "replacement" }, { 4, "second four" }, { 3, "three" }
		};
		map.insert(incoming.begin(), incoming.end());

		CHECK((map.keys() == std::vector<int>{ 1, 2, 3, 4, 5 }));
		CHECK((map.values() == std::vector<std::string>{ "one", "existing", "three", "four", "five" }));
	}

	SECTION("an empty range changes nothing")
	{
		const std::vector<std::pair<int, std::string>> empty;
		map.insert(empty.begin(), empty.end());
		CHECK((map.keys() == std::vector<int>{ 2, 5 }));
	}

	SECTION("a moved-from range hands its values over")
	{
		std::vector<std::pair<int, std::string>> incoming{ { 3, "three" }, { 1, "one" } };
		map.insert(std::make_move_iterator(incoming.begin()), std::make_move_iterator(incoming.end()));

		CHECK((map.keys() == std::vector<int>{ 1, 2, 3, 5 }));
		CHECK(map.at(3) == "three");
	}

	SECTION("the single-element overload still resolves")
	{
		CHECK(map.insert(std::pair<int, std::string>{ 7, "seven" }).second);
		CHECK((map.keys() == std::vector<int>{ 2, 5, 7 }));
	}
}

TEST_CASE("flat_map::insert restores the map when an element throws", "[flat-map]")
{
	flat_map<int, throwing_value> map;
	map.try_emplace(1, 10);
	map.try_emplace(3, 30);

	const std::vector<std::pair<int, int>> incoming{ { 2, 20 }, { 4, throwing_value::poison } };
	CHECK_THROWS_AS(map.insert(incoming.begin(), incoming.end()), throwing_value::construction_failed);

	CHECK_FALSE(map.batch_open());
	REQUIRE(map.size() == 2);
	CHECK(map.at(1).value == 10);
	CHECK(map.at(3).value == 30);
	// Appended before the throw, and discarded with the rest of the batch
	CHECK(map.find(2) == map.end());
}

TEST_CASE("flat_map operator[] inserts on a miss and preserves the value on a hit", "[flat-map]")
{
	flat_map<int, int> map;

	CHECK(map[1] == 0); // value-initialized on insertion
	CHECK(map.size() == 1);

	map[1] = 10;
	CHECK(map[1] == 10); // a second subscript returns the stored value instead of resetting it
	CHECK(map.size() == 1);

	map[5] = 50;
	map[3] = 30;
	CHECK((map.keys() == std::vector<int>{ 1, 3, 5 }));
	CHECK((map.values() == std::vector<int>{ 10, 30, 50 }));

	flat_map<std::string, int> keyed;
	std::string key{ "a key long enough to defeat the small string optimization" };
	keyed[std::move(key)] = 7;
	CHECK(keyed.at("a key long enough to defeat the small string optimization") == 7);
	CHECK(key.empty());
}

TEST_CASE("flat_map iterators refuse reordering algorithms", "[flat-map]")
{
	using iterator = flat_map<int, int>::iterator;

	// Keys stay immutable, so nothing can be assigned through the proxy
	static_assert(!std::indirectly_writable<iterator, std::pair<int, int>>);
	static_assert(!std::permutable<iterator>);
	static_assert(!std::sortable<iterator>);

	static_assert(std::random_access_iterator<iterator>);
	static_assert(std::indirectly_readable<iterator>);
}

TEST_CASE("flat_set appends strictly ordered unique values without consuming rejected values", "[flat-set]")
{
	flat_set<move_only_value> set;
	set.reserve(3);
	CHECK(set.append_sorted_unique(move_only_value(10)));
	CHECK(set.append_sorted_unique(move_only_value(30)));

	move_only_value duplicate(30);
	CHECK_FALSE(set.append_sorted_unique(std::move(duplicate)));
	CHECK(duplicate.value == 30);

	move_only_value outOfOrder(20);
	CHECK_FALSE(set.append_sorted_unique(std::move(outOfOrder)));
	CHECK(outOfOrder.value == 20);

	REQUIRE(set.size() == 2);
	CHECK(set.begin()->value == 10);
	CHECK((set.begin() + 1)->value == 30);
}

TEST_CASE("flat_set batch insertion keeps the first of several equivalent values", "[flat-set]")
{
	// Past the insertion-sort threshold: with fewer entries an unstable sort would preserve order by accident
	constexpr int batchSize = 40;

	SECTION("sorted in place, with no existing entries")
	{
		flat_set<tagged_value, tagged_less> set;

		set.begin_batch();
		for (int tag = 0; tag < batchSize; ++tag)
			set.append_unsorted(tagged_value{ tag % 2, tag });
		set.end_batch();

		REQUIRE(set.size() == 2);
		CHECK(set.begin()->tag == 0);
		CHECK((set.begin() + 1)->tag == 1);
	}

	SECTION("merged into existing entries")
	{
		flat_set<tagged_value, tagged_less> set;
		set.insert(tagged_value{ 9, -1 });

		set.begin_batch();
		for (int tag = 0; tag < batchSize; ++tag)
			set.append_unsorted(tagged_value{ tag % 2, tag });
		set.end_batch();

		REQUIRE(set.size() == 3);
		CHECK(set.begin()->tag == 0);
		CHECK((set.begin() + 1)->tag == 1);
		CHECK((set.begin() + 2)->tag == -1);
	}
}

TEST_CASE("flat_set supports ordinary, sorted bulk, and batch insertion", "[flat-set]")
{
	flat_set<int> set{ 5, 2, 2 };
	CHECK(set.insert(3).second);
	CHECK_FALSE(set.insert(3).second);

	const std::vector<int> sorted{ 1, 2, 4, 4, 6 };
	set.insert_sorted(sorted.begin(), sorted.end());

	set.begin_batch();
	set.append_unsorted(8);
	set.append_unsorted(0);
	set.append_unsorted(7);
	set.append_unsorted(8);
	set.end_batch();

	const std::vector<int> expected{ 0, 1, 2, 3, 4, 5, 6, 7, 8 };
	CHECK(std::equal(set.begin(), set.end(), expected.begin(), expected.end()));
}

TEST_CASE("flat_set exposes its keys as a vector", "[flat-set]")
{
	flat_set<int> set{ 3, 1, 2 };
	static_assert(std::is_same_v<decltype(set.keys()), const std::vector<int>&>);

	CHECK((set.keys() == std::vector<int>{ 1, 2, 3 }));

	SECTION("a cleared set yields an empty view")
	{
		set.clear();
		CHECK(set.keys().empty());
	}

	SECTION("insertion and erasure are reflected in order")
	{
		set.insert(0);
		set.erase(2);
		CHECK((set.keys() == std::vector<int>{ 0, 1, 3 }));
	}

	SECTION("a merged batch shows up sorted and deduplicated")
	{
		set.begin_batch();
		set.append_unsorted(5);
		set.append_unsorted(0);
		set.append_unsorted(5);
		set.end_batch();
		CHECK((set.keys() == std::vector<int>{ 0, 1, 2, 3, 5 }));
	}
}

TEST_CASE("flat_set iterates in reverse", "[flat-set]")
{
	flat_set<int> set{ 3, 1, 2 };

	const std::vector<int> descending{ 3, 2, 1 };
	CHECK(std::equal(set.rbegin(), set.rend(), descending.begin(), descending.end()));
	CHECK(std::equal(set.crbegin(), set.crend(), descending.begin(), descending.end()));

	CHECK(*set.rbegin() == 3);
	CHECK((set.rend() - set.rbegin()) == 3);
	CHECK(set.rbegin().base() == set.end());

	set.clear();
	CHECK(set.rbegin() == set.rend());
}

TEST_CASE("flat_set exposes the first and last keys", "[flat-set]")
{
	flat_set<int> set{ 3, 1, 2 };
	static_assert(std::is_same_v<decltype(set.front()), const int&>);

	CHECK(set.front() == 1);
	CHECK(set.back() == 3);

	set.insert(0);
	set.insert(9);
	CHECK(set.front() == 0);
	CHECK(set.back() == 9);

	const flat_set<int> single{ 7 };
	CHECK(single.front() == 7);
	CHECK(single.back() == 7);
}

TEST_CASE("flat_set inserts an unsorted range", "[flat-set]")
{
	flat_set<int> set{ 2, 5 };

	SECTION("the range is sorted, deduplicated and merged")
	{
		const std::vector<int> incoming{ 4, 1, 2, 4, 3 };
		set.insert(incoming.begin(), incoming.end());
		CHECK((set.keys() == std::vector<int>{ 1, 2, 3, 4, 5 }));
	}

	SECTION("an empty range changes nothing")
	{
		const std::vector<int> empty;
		set.insert(empty.begin(), empty.end());
		CHECK((set.keys() == std::vector<int>{ 2, 5 }));
	}

	SECTION("an input iterator range needs no distance")
	{
		std::istringstream numbers{ "4 1 3" };
		set.insert(std::istream_iterator<int>(numbers), std::istream_iterator<int>());
		CHECK((set.keys() == std::vector<int>{ 1, 2, 3, 4, 5 }));
	}

	SECTION("the single-element overload still resolves")
	{
		CHECK(set.insert(7).second);
		CHECK((set.keys() == std::vector<int>{ 2, 5, 7 }));
	}
}

TEST_CASE("flat containers answer every lookup on an empty container", "[flat-map][flat-set]")
{
	flat_map<int, int> map;
	CHECK(map.empty());
	CHECK(map.size() == 0);
	CHECK(map.begin() == map.end());
	CHECK(map.cbegin() == map.cend());
	CHECK(map.rbegin() == map.rend());
	CHECK(map.find(1) == map.end());
	CHECK_FALSE(map.contains(1));
	CHECK(map.count(1) == 0);
	CHECK(map.lower_bound(1) == map.end());
	CHECK(map.upper_bound(1) == map.end());
	CHECK(map.erase(1) == 0);
	CHECK_THROWS_AS(map.at(1), std::out_of_range);
	CHECK(map.keys().empty());
	CHECK(map.values().empty());

	flat_set<int> set;
	CHECK(set.empty());
	CHECK(set.size() == 0);
	CHECK(set.begin() == set.end());
	CHECK(set.cbegin() == set.cend());
	CHECK(set.rbegin() == set.rend());
	CHECK(set.find(1) == set.end());
	CHECK_FALSE(set.contains(1));
	CHECK(set.count(1) == 0);
	CHECK(set.lower_bound(1) == set.end());
	CHECK(set.upper_bound(1) == set.end());
	CHECK(set.erase(1) == 0);
	CHECK(set.keys().empty());
}

TEST_CASE("clear() ends an open batch", "[flat-map][flat-set]")
{
	flat_map<int, int> map{ { 1, 10 } };
	map.begin_batch();
	map.append_unsorted(2, 20);
	REQUIRE(map.batch_open());

	map.clear();
	CHECK_FALSE(map.batch_open());
	CHECK(map.empty());
	// No batch is left open, so the ordered operations are available again
	map.try_emplace(3, 30);
	CHECK(map.at(3) == 30);

	flat_set<int> set{ 1 };
	set.begin_batch();
	set.append_unsorted(2);
	REQUIRE(set.batch_open());

	set.clear();
	CHECK_FALSE(set.batch_open());
	CHECK(set.empty());
	CHECK(set.insert(3).second);
	CHECK(*set.begin() == 3);
}

TEST_CASE("flat containers erase degenerate ranges", "[flat-map][flat-set]")
{
	flat_map<int, int> map{ { 1, 10 }, { 2, 20 }, { 3, 30 } };

	auto position = map.erase(map.begin(), map.begin());
	CHECK(position == map.begin());
	CHECK(map.size() == 3);

	position = map.erase(map.end(), map.end());
	CHECK(position == map.end());
	CHECK(map.size() == 3);

	position = map.erase(map.begin(), map.end());
	CHECK(position == map.end());
	CHECK(map.empty());

	flat_set<int> set{ 1, 2, 3 };
	CHECK(set.erase(set.begin(), set.begin()) == set.begin());
	CHECK(set.size() == 3);
	CHECK(set.erase(set.begin(), set.end()) == set.end());
	CHECK(set.empty());
}

TEST_CASE("flat containers handle degenerate batches", "[flat-map][flat-set]")
{
	SECTION("every appended key already exists")
	{
		flat_map<int, std::string> map{ { 1, "one" }, { 2, "two" } };
		map.begin_batch();
		map.append_unsorted(2, "loses");
		map.append_unsorted(1, "also loses");
		map.end_batch();

		CHECK((map.keys() == std::vector<int>{ 1, 2 }));
		CHECK((map.values() == std::vector<std::string>{ "one", "two" }));
	}

	SECTION("every appended key is the same one")
	{
		flat_set<int> set;
		set.begin_batch();
		// Past the insertion-sort threshold, so the sort really has to be stable
		for (int index = 0; index < 40; ++index)
			set.append_unsorted(7);
		set.end_batch();

		CHECK((set.keys() == std::vector<int>{ 7 }));
	}

	SECTION("the batch sorts entirely below the existing prefix")
	{
		flat_map<int, int> map{ { 10, 100 }, { 20, 200 } };
		map.begin_batch();
		map.append_unsorted(2, 20);
		map.append_unsorted(1, 10);
		map.end_batch();

		CHECK((map.keys() == std::vector<int>{ 1, 2, 10, 20 }));
		CHECK((map.values() == std::vector<int>{ 10, 20, 100, 200 }));
	}

	SECTION("two batches in sequence")
	{
		flat_set<int> set{ 5 };
		set.begin_batch();
		set.append_unsorted(3);
		set.end_batch();

		set.begin_batch();
		set.append_unsorted(4);
		set.append_unsorted(3);
		set.end_batch();

		CHECK((set.keys() == std::vector<int>{ 3, 4, 5 }));
	}
}

TEST_CASE("flat containers insert an initializer list", "[flat-map][flat-set]")
{
	flat_map<int, std::string> map{ { 2, "existing" } };
	map.insert({ { 4, "four" }, { 1, "one" }, { 2, "replacement" }, { 4, "second four" } });
	CHECK((map.keys() == std::vector<int>{ 1, 2, 4 }));
	CHECK((map.values() == std::vector<std::string>{ "one", "existing", "four" }));

	flat_set<int> set{ 2 };
	set.insert({ 4, 1, 2, 4 });
	CHECK((set.keys() == std::vector<int>{ 1, 2, 4 }));

	// A lone element still selects the element overload over the list one
	CHECK(map.insert(std::pair<int, std::string>{ 9, "nine" }).second);
	CHECK(set.insert(9).second);
}

TEST_CASE("flat containers copy and move", "[flat-map][flat-set]")
{
	const flat_map<int, std::string> original{ { 2, "two" }, { 1, "one" } };

	flat_map<int, std::string> copied = original;
	CHECK(copied == original);
	CHECK_FALSE(copied.batch_open());

	const flat_map<int, std::string> moved = std::move(copied);
	CHECK(moved == original);

	flat_map<int, std::string> assigned;
	assigned = original;
	assigned.try_emplace(3, "three");
	CHECK(assigned.size() == 3);
	CHECK(original.size() == 2);

	const flat_set<int> original_set{ 2, 1 };
	flat_set<int> copied_set = original_set;
	CHECK(copied_set == original_set);

	const flat_set<int> moved_set = std::move(copied_set);
	CHECK(moved_set == original_set);
}

TEST_CASE("flat containers shrink to fit and report their comparators", "[flat-map][flat-set]")
{
	flat_map<int, int> map;
	map.reserve(100);
	map.try_emplace(1, 10);
	map.try_emplace(2, 20);
	map.shrink_to_fit();
	CHECK((map.keys() == std::vector<int>{ 1, 2 }));
	CHECK((map.values() == std::vector<int>{ 10, 20 }));
	CHECK(map.key_comp()(1, 2));

	flat_set<int> set;
	set.reserve(100);
	set.insert(1);
	set.shrink_to_fit();
	CHECK((set.keys() == std::vector<int>{ 1 }));
	CHECK(set.key_comp()(1, 2));
	CHECK(set.value_comp()(1, 2));
}

TEST_CASE("flat_set supports heterogeneous lookup", "[flat-set]")
{
	flat_set<std::string> set{ "one", "three", "two" };

	CHECK(*set.find(std::string_view("two")) == "two");
	CHECK(set.find(std::string_view("four")) == set.end());
	CHECK(set.contains(std::string_view("one")));
	CHECK_FALSE(set.contains(std::string_view("four")));
	CHECK(set.count(std::string_view("three")) == 1);
	CHECK(set.count(std::string_view("four")) == 0);
	CHECK(*set.lower_bound(std::string_view("t")) == "three");
	CHECK(*set.upper_bound(std::string_view("three")) == "two");
	CHECK(set.erase(std::string_view("one")) == 1);
	CHECK(set.size() == 2);
}

TEST_CASE("flat_set::insert restores the set when an element throws", "[flat-set]")
{
	flat_set<throwing_value, throwing_less> set;
	set.insert(throwing_value(1));
	set.insert(throwing_value(3));

	const std::vector<int> incoming{ 2, throwing_value::poison };
	CHECK_THROWS_AS(set.insert(incoming.begin(), incoming.end()), throwing_value::construction_failed);

	CHECK_FALSE(set.batch_open());
	REQUIRE(set.size() == 2);
	CHECK(set.begin()->value == 1);
	CHECK((set.begin() + 1)->value == 3);
}

TEST_CASE("end_batch leaves the batch open when it throws", "[flat-map][flat-set]")
{
	SECTION("flat_map")
	{
		flat_map<int, int, armable_less> map;
		map.try_emplace(1, 10);
		map.try_emplace(3, 30);

		map.begin_batch();
		map.append_unsorted(2, 20);
		map.append_unsorted(4, 40);

		armable_less::comparisons_until_throw = 0;
		CHECK_THROWS_AS(map.end_batch(), comparison_failed);
		armable_less::comparisons_until_throw = -1;

		REQUIRE(map.batch_open());
		map.abort_batch();

		CHECK_FALSE(map.batch_open());
		CHECK((map.keys() == std::vector<int>{ 1, 3 }));
		CHECK((map.values() == std::vector<int>{ 10, 30 }));
	}

	SECTION("flat_set")
	{
		flat_set<int, armable_less> set;
		set.insert(1);
		set.insert(3);

		set.begin_batch();
		set.append_unsorted(2);
		set.append_unsorted(4);

		armable_less::comparisons_until_throw = 0;
		CHECK_THROWS_AS(set.end_batch(), comparison_failed);
		armable_less::comparisons_until_throw = -1;

		REQUIRE(set.batch_open());
		set.abort_batch();

		CHECK_FALSE(set.batch_open());
		CHECK((set.keys() == std::vector<int>{ 1, 3 }));
	}
}

TEST_CASE("flat containers return an equal range spanning at most one entry", "[flat-map][flat-set]")
{
	flat_map<int, int> map{ { 1, 10 }, { 3, 30 }, { 5, 50 } };

	auto [first, last] = map.equal_range(3);
	CHECK((last - first) == 1);
	CHECK(first->first == 3);
	CHECK(first->second == 30);
	CHECK(last == map.find(5));

	// A miss returns an empty range positioned where the key would go
	std::tie(first, last) = map.equal_range(4);
	CHECK(first == last);
	CHECK(first == map.lower_bound(4));
	std::tie(first, last) = map.equal_range(0);
	CHECK(first == last);
	CHECK(first == map.begin());
	std::tie(first, last) = map.equal_range(9);
	CHECK(first == last);
	CHECK(first == map.end());

	const auto& const_map = map;
	const auto [const_first, const_last] = const_map.equal_range(5);
	CHECK((const_last - const_first) == 1);
	CHECK(const_first->second == 50);
	CHECK(const_last == const_map.end());

	const flat_map<int, int> empty_map;
	CHECK(empty_map.equal_range(1).first == empty_map.end());

	flat_set<std::string> set{ "one", "three", "two" };
	const auto [set_first, set_last] = set.equal_range(std::string_view("three"));
	CHECK((set_last - set_first) == 1);
	CHECK(*set_first == "three");
	CHECK(set.equal_range(std::string_view("four")).first == set.lower_bound(std::string_view("four")));
}

TEST_CASE("flat containers swap", "[flat-map][flat-set]")
{
	flat_map<int, int, directional_less> ascending{ { 1, 10 }, { 2, 20 } };
	flat_map<int, int, directional_less> descending({ { 1, 10 }, { 2, 20 } }, directional_less{ true });
	REQUIRE((ascending.keys() == std::vector<int>{ 1, 2 }));
	REQUIRE((descending.keys() == std::vector<int>{ 2, 1 }));

	SECTION("the member form exchanges contents and comparators")
	{
		ascending.swap(descending);
		CHECK((ascending.keys() == std::vector<int>{ 2, 1 }));
		CHECK((descending.keys() == std::vector<int>{ 1, 2 }));
		// The comparator came along, so lookups still work in each container's own order
		CHECK(ascending.find(2) == ascending.begin());
		CHECK(descending.find(2) == descending.begin() + 1);
	}

	SECTION("the free form is found by argument-dependent lookup")
	{
		using std::swap;
		swap(ascending, descending);
		CHECK((ascending.keys() == std::vector<int>{ 2, 1 }));
		CHECK((descending.keys() == std::vector<int>{ 1, 2 }));
	}

	SECTION("flat_set swaps too")
	{
		flat_set<int> left{ 1, 2 };
		flat_set<int> right{ 8, 9 };
		using std::swap;
		swap(left, right);
		CHECK((left.keys() == std::vector<int>{ 8, 9 }));
		CHECK((right.keys() == std::vector<int>{ 1, 2 }));
	}

	SECTION("an open batch survives the swap")
	{
		flat_set<int> batching{ 1 };
		flat_set<int> plain{ 5 };
		batching.begin_batch();
		batching.append_unsorted(0);

		using std::swap;
		swap(batching, plain);
		CHECK_FALSE(batching.batch_open());
		REQUIRE(plain.batch_open());

		plain.end_batch();
		CHECK((plain.keys() == std::vector<int>{ 0, 1 }));
		CHECK((batching.keys() == std::vector<int>{ 5 }));
	}
}

TEST_CASE("flat containers order lexicographically", "[flat-map][flat-set]")
{
	using map_type = flat_map<int, int>;
	static_assert(std::three_way_comparable<map_type>);
	static_assert(std::same_as<decltype(map_type{} <=> map_type{}), std::strong_ordering>);

	SECTION("a differing key decides before any mapped value")
	{
		// Comparing the key vectors and then the value vectors would call this greater, not less
		const map_type left{ { 1, 0 }, { 3, 0 } };
		const map_type right{ { 1, 5 }, { 2, 0 } };
		CHECK(left < right);
		CHECK(right > left);
	}

	SECTION("a mapped value decides when the keys match")
	{
		CHECK((map_type{ { 1, 10 } } < map_type{ { 1, 20 } }));
		CHECK((map_type{ { 1, 20 } } > map_type{ { 1, 10 } }));
	}

	SECTION("a prefix is less than the longer container")
	{
		CHECK((map_type{ { 1, 10 } } < map_type{ { 1, 10 }, { 2, 20 } }));
		CHECK((map_type{} < map_type{ { 1, 10 } }));
		CHECK((map_type{} <=> map_type{}) == std::strong_ordering::equal);
	}

	SECTION("equivalent containers compare equal and the relational operators agree")
	{
		const map_type left{ { 1, 10 }, { 2, 20 } };
		const map_type right{ { 2, 20 }, { 1, 10 } };
		CHECK((left <=> right) == std::strong_ordering::equal);
		CHECK(left <= right);
		CHECK(left >= right);
		CHECK_FALSE(left < right);
		CHECK_FALSE(left > right);
	}

	SECTION("flat_set orders on keys alone")
	{
		CHECK((flat_set<int>{ 1, 2 } < flat_set<int>{ 1, 3 }));
		CHECK((flat_set<int>{ 1 } < flat_set<int>{ 1, 2 }));
		CHECK((flat_set<int>{ 2, 1 } <=> flat_set<int>{ 1, 2 }) == std::strong_ordering::equal);
	}

	SECTION("a mapped type carrying only operator< still orders, through weak_ordering")
	{
		using ordered_map = flat_map<int, move_only_value>;
		static_assert(std::same_as<decltype(std::declval<const ordered_map&>() <=> std::declval<const ordered_map&>()), std::weak_ordering>);

		ordered_map left;
		ordered_map right;
		CHECK(left.append_sorted_unique(1, move_only_value(10)));
		CHECK(right.append_sorted_unique(1, move_only_value(20)));
		CHECK(left < right);
	}
}

TEST_CASE("comparison operators stay out of the way of comparator-only keys", "[flat-map][flat-set]")
{
	// ordered_only_key carries none of ==, < or <=>, so the containers must simply not be comparable either way
	static_assert(!std::three_way_comparable<flat_map<ordered_only_key, int, ordered_only_less>>);
	static_assert(!std::three_way_comparable<flat_set<ordered_only_key, ordered_only_less>>);
	static_assert(!std::equality_comparable<flat_map<ordered_only_key, int, ordered_only_less>>);
	static_assert(!std::equality_comparable<flat_set<ordered_only_key, ordered_only_less>>);

	static_assert(std::equality_comparable<flat_map<int, int>>);
	static_assert(std::three_way_comparable<flat_map<int, int>>);

	// move_only_value orders with < but carries no ==, so the map orders without being equality comparable
	using orderable_map = flat_map<int, move_only_value>;
	static_assert(!std::equality_comparable<orderable_map>);
	static_assert(!std::three_way_comparable<orderable_map>); // Bundles equality in, so the missing == fails it
	static_assert(std::same_as<decltype(std::declval<const orderable_map&>() <=> std::declval<const orderable_map&>()), std::weak_ordering>);

	// Still perfectly usable for everything else
	flat_map<ordered_only_key, int, ordered_only_less> map;
	map.try_emplace(ordered_only_key{ 2 }, 20);
	map.try_emplace(ordered_only_key{ 1 }, 10);
	CHECK(map.size() == 2);
	CHECK(map.find(ordered_only_key{ 1 })->second == 10);
}
