#include "3rdparty/catch2/catch.hpp"
#include "container/flat_map.hpp"

#include <algorithm>
#include <iterator>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

	struct grouped_key
	{
		int group;
		int identity;

		friend bool operator==(const grouped_key&, const grouped_key&) = default;
	};

	struct group_less
	{
		bool operator()(const grouped_key& left, const grouped_key& right) const { return left.group < right.group; }
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

TEST_CASE("flat_map insertion and lookup use equality within an ordering-equivalent group", "[flat-map]")
{
	flat_map<grouped_key, std::string, group_less> map;
	CHECK(map.try_emplace(grouped_key{ 1, 10 }, "ten").second);
	CHECK(map.try_emplace(grouped_key{ 1, 20 }, "twenty").second);
	CHECK_FALSE(map.try_emplace(grouped_key{ 1, 10 }, "replacement").second);

	CHECK(map.size() == 2);
	CHECK(map.at(grouped_key{ 1, 10 }) == "ten");
	CHECK(map.at(grouped_key{ 1, 20 }) == "twenty");
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

TEST_CASE("flat_set uses equality within an ordering-equivalent group", "[flat-set]")
{
	flat_set<grouped_key, group_less> set;
	CHECK(set.insert(grouped_key{ 1, 10 }).second);
	CHECK(set.insert(grouped_key{ 1, 20 }).second);
	CHECK_FALSE(set.insert(grouped_key{ 1, 10 }).second);
	CHECK(set.contains(grouped_key{ 1, 20 }));
}
