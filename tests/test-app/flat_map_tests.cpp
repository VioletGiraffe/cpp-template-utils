#include "3rdparty/catch2/catch.hpp"
#include "container/flat_map.hpp"

#include <algorithm>
#include <concepts>
#include <iterator>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
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

		int value;
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
