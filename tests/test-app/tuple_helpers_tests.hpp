#pragma once

#include "3rdparty/catch2/catch.hpp"
#include "tuple/tuple_helpers.hpp"
#include "utility/constexpr_algorithms.hpp"

#include <limits>
#include <string>
#include <variant>

TEST_CASE("index_for_type", "[tuple]") {
	static_assert (tuple::indexForType<int>(std::tuple<int>{}) == 0);
	static_assert (tuple::indexForType<int>(std::tuple<int, float>{}) == 0);
	static_assert (tuple::indexForType<int>(std::tuple<int, float, std::tuple<>>{}) == 0);
	static_assert (tuple::indexForType<float>(std::tuple<int, float, std::tuple<>>{}) == 1);
	static_assert (tuple::indexForType<std::tuple<>>(std::tuple<int, float, std::tuple<>>{}) == 2);
	static_assert (tuple::indexForType<std::tuple<>>(std::tuple<std::tuple<>, int, float, std::tuple<>>{}) == 0);

	CHECK(tuple::indexForType<std::tuple<>>(std::tuple<std::tuple<>, int, float, std::tuple<>>{}) == 0);
}

TEST_CASE("visit", "[tuple]") {
	SECTION("const") {
		constexpr std::tuple reference{-1, std::numeric_limits<uint64_t>::max(), 1.7f};
		tuple::visit(reference, 0, [](auto&& item){
			CHECK(item == -1);
		});

		tuple::visit(reference, 1, [](auto&& item){
			CHECK(item == std::numeric_limits<uint64_t>::max());
			CHECK(item != std::numeric_limits<uint64_t>::max() - 1);
		});

		tuple::visit(reference, 2, [](auto&& item){
			CHECK(item == 1.7f);
			CHECK(item != 1.65f);
		});
	}

	SECTION("mutable") {
		std::tuple reference{-1, std::numeric_limits<uint64_t>::max(), 1.7f};
		const auto r2 = reference;
		tuple::visit(reference, 0, [](auto&& item){
			item = -1043;
		});

		tuple::visit(reference, 1, [](auto&& item){
			item = std::numeric_limits<uint64_t>::max() - 5;
		});

		tuple::visit(reference, 2, [](auto&& item){
			item = 3.14159f;
		});

		CHECK(reference == std::tuple{-1043, std::numeric_limits<uint64_t>::max() - 5, 3.14159f});
		CHECK(reference != r2);
	}
}

std::string& operator+=(std::string& str, int i) {
	return str += std::to_string(i);
}

std::string operator+(const std::string& str, int i) {
	return str + std::to_string(i);
}

TEST_CASE("for_each", "[tuple]") {

	SECTION("const") {
		const std::tuple reference{-1, std::numeric_limits<uint64_t>::max(), std::string{"abc"}, 1.7f};
		using VariantType = std::variant<int, uint64_t, std::string, float>;
		std::vector<VariantType> items;

		tuple::for_each(reference, [&](auto&& item) {
			items.emplace_back(item);
		});

		static_for<0, std::tuple_size_v<decltype(reference)>>([&](auto index_wrapper){
			CHECK(VariantType{std::get<index_wrapper>(reference)} == items[index_wrapper]);
		});
	}

	SECTION("mutable") {
		std::tuple reference{-1, std::numeric_limits<uint64_t>::max(), std::string{"abc"}, 1.7f};
		const auto r2 = reference;

		tuple::for_each(reference, [&](auto&& item) {
			item += 1;
		});

		static_for<0, std::tuple_size_v<decltype(reference)>>([&](auto index_wrapper){
			CHECK(std::get<index_wrapper>(reference) != std::get<index_wrapper>(r2));
			CHECK(std::get<index_wrapper>(reference) == std::get<index_wrapper>(r2) + 1);
		});

		CHECK(std::get<2>(reference) == "abc1");
	}
}
