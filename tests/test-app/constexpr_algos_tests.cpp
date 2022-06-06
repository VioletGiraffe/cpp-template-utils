#include "3rdparty/catch2/catch.hpp"
#include "utility/constexpr_algorithms.hpp"
#include "tuple/tuple_helpers.hpp"

#include <array>
#include <numeric>

TEST_CASE("constexpr_for_z (size_t)", "[constexpr-algos]")
{
	// Range backwards
	{
		std::vector<size_t> v;
		static_for<5, 0>([&v](auto index) {
			static_assert(std::is_same_v<typename decltype(index)::type, size_t>);
			v.push_back(index);
		});

		CHECK(v.empty());
	}

	// Empty range
	{
		std::vector<size_t> v;
		static_for<1, 1>([&v](auto index) {
			static_assert(std::is_same_v<typename decltype(index)::type, size_t>);
			v.push_back(index);
		});

		static_for<0, 0>([&v](auto index) {
			static_assert(std::is_same_v<typename decltype(index)::type, size_t>);
			v.push_back(index);
		});

		CHECK(v.empty());
	}

	// Positive range
	{
		std::vector<size_t> v;
		static_for<0, 5>([&v](auto index) {
			static_assert(std::is_same_v<typename decltype(index)::type, size_t>);
			v.push_back(index);
		});

		CHECK(v.size() == 5);
		std::vector<size_t> testVector(5, 0);
		std::iota(testVector.begin(), testVector.end(), 0);
		CHECK(v == testVector);
	}
}

TEST_CASE("constexpr_for_z<int>", "[constexpr-algos]")
{
	// Range backwards
	{
		std::vector<int> v;
		constexpr_for_i<0, -5>([&v](auto index) {
			static_assert(std::is_same_v<typename decltype(index)::type, int>);
			v.push_back(index);
		});

		constexpr_for_i<2, 1>([&v](auto index) {
			static_assert(std::is_same_v<typename decltype(index)::type, int>);
			v.push_back(index);
		});

		constexpr_for_i<-1, -5>([&v](auto index) {
			static_assert(std::is_same_v<typename decltype(index)::type, int>);
			v.push_back(index);
		});

		constexpr_for_i<1, -5>([&v](auto index) {
			static_assert(std::is_same_v<typename decltype(index)::type, int>);
			v.push_back(index);
		});

		CHECK(v.empty());
	}

	// Empty range
	{
		std::vector<int> v;
		constexpr_for_i<1, 1>([&v](auto index) {
			static_assert(std::is_same_v<typename decltype(index)::type, int>);
			v.push_back(index);
		});

		constexpr_for_i<0, 0>([&v](auto index) {
			static_assert(std::is_same_v<typename decltype(index)::type, int>);
			v.push_back(index);
		});

		constexpr_for_i<-1, -1>([&v](auto index) {
			static_assert(std::is_same_v<typename decltype(index)::type, int>);
			v.push_back(index);
		});

		CHECK(v.empty());
	}

	// Positive range
	{
		std::vector<int> v;
		constexpr_for_i<0, 5>([&v](auto index) {
			static_assert(std::is_same_v<typename decltype(index)::type, int>);
			v.push_back(index);
		});

		CHECK(v.size() == 5);
		std::vector<int> testVector(5, 0);
		std::iota(testVector.begin(), testVector.end(), 0);
		CHECK(v == testVector);
	}

	// Negative range
	{
		std::vector<int> v;
		constexpr_for_i<-4, 1>([&v](auto index) {
			static_assert(std::is_same_v<typename decltype(index)::type, int>);
			v.push_back(index);
		});

		CHECK(v.size() == 5);
		std::vector<int> testVector(5, 0);
		CHECK(v != testVector);
		std::iota(testVector.begin(), testVector.end(), -4);
		CHECK(v == testVector);
	}


	// Symmetric range
	{
		std::vector<int> v;
		constexpr_for_i<-4, 5>([&v](auto index) {
			v.push_back(index);
		});

		CHECK(v.size() == 9);
		std::vector<int> testVector(9, 0);
		CHECK(v != testVector);
		std::iota(testVector.begin(), testVector.end(), -4);
		CHECK(v == testVector);
	}
}

TEST_CASE("constexpr_for_z - compile-time operation", "[constexpr-algos]")
{
	constexpr_for_z<0, 1>([](auto index) {
		static_assert(std::is_same_v<typename decltype(index)::type, size_t>);
		static_assert(index == 0);
	});

	constexpr_for_z<2, 3>([](auto index) {
		static_assert(std::is_same_v<typename decltype(index)::type, size_t>);
		static_assert(index != 0);
		static_assert(index == 2);
	});

	static constexpr auto t = std::tuple{ 'a', 3.14f, 15};
	constexpr_for_z<0, tuple::size(t)>([](auto index) {
		static_assert(std::is_same_v<typename decltype(index)::type, size_t>);

		if constexpr (index == 0)
			static_assert(std::get<index>(t) == 'a');
		else if constexpr (index == 1)
			static_assert(std::get<index>(t) == 3.14f);
		else if constexpr (index == 2)
			static_assert(std::get<index>(t) == 15);
		else
			static_assert(false_v<decltype(index)>, "This shouldn't happen!");
	});

	SUCCEED();
}

template <class F>
consteval void eval(F&& f) {
	f();
}

TEST_CASE("constexpr_for_z -> bool - early exit at runtime", "[constexpr-algos]")
{
	std::vector<size_t> v;
	constexpr_for_z<5, 200>([&v](auto index) -> bool {
		static_assert(std::is_same_v<typename decltype(index)::type, size_t>);
		v.push_back(index);
		return index < 7 ? true : false;
	});

	CHECK(v == std::vector<size_t>{5, 6, 7});
}