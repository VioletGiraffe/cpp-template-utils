#pragma once

#include "3rdparty/catch2/catch.hpp"
#include "utility/constexpr_algorithms.hpp"

#include <array>
#include <numeric>

TEST_CASE("static_for", "[constexpr-algos]") {

	SECTION("Range backwards") {
		std::vector<int> v;
		static_for<0, -5>([&v](auto index) {
			v.emplace_back(index);
		});

		CHECK(v.empty());
	}

	SECTION("Empty range") {
		std::vector<int> v;
		static_for<1, 1>([&v](auto index) {
			v.emplace_back(index);
		});

		CHECK(v.empty());
	}

	SECTION("Positive range") {
		std::vector<int> v;
		static_for<0, 5>([&v](auto index) {
			v.emplace_back(index);
		});

		CHECK(v.size() == 5);
		std::vector<int> testVector(5, 0);
		std::iota(testVector.begin(), testVector.end(), 0);
		CHECK(v == testVector);
	}

	SECTION("Negative range") {
		std::vector<int> v;
		static_for<-4, 1>([&v](auto index) {
			v.emplace_back(index);
		});

		CHECK(v.size() == 5);
		std::vector<int> testVector(5, 0);
		CHECK(v != testVector);
		std::iota(testVector.begin(), testVector.end(), -4);
		CHECK(v == testVector);
	}


	SECTION("Symmetric range") {
		std::vector<int> v;
		static_for<-4, 5>([&v](auto index) {
			v.emplace_back(index);
		});

		CHECK(v.size() == 9);
		std::vector<int> testVector(9, 0);
		CHECK(v != testVector);
		std::iota(testVector.begin(), testVector.end(), -4);
		CHECK(v == testVector);
	}
}
