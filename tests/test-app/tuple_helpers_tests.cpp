#include "3rdparty/catch2/catch.hpp"
#include "tuple/tuple_helpers.hpp"
#include "utility/constexpr_algorithms.hpp"
#include "utility/extra_type_traits.hpp"
#include "compiler/compiler_warnings_control.h"
#include "string/string_helpers.hpp"

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

TEST_CASE("for_each", "[tuple]") {

	{ // Const
		const std::tuple reference{-1, std::numeric_limits<uint64_t>::max(), std::string{"abc"}, 1.7f};
		using VariantType = std::variant<int, uint64_t, std::string, float>;
		std::vector<VariantType> items;

		tuple::for_each(reference, [&](auto&& item) {
			items.emplace_back(item);
		});

		static_for<0, std::tuple_size_v<decltype(reference)>>([&]<auto index> (){
			CHECK(VariantType{std::get<index>(reference)} == items[index]);
		});
	}

	{ // Mutable
		std::tuple reference{-1, std::numeric_limits<uint64_t>::max(), std::string{"abc"}, 3.14f};
		const auto r2 = reference;

		tuple::for_each(reference, [&](auto&& item) {
			using ItemType = remove_cv_and_reference_t<decltype(item)>;
			if constexpr (std::is_floating_point_v<ItemType>)
				item += 2.5f;
			else
				item += 1;
		});

		static_for<0, std::tuple_size_v<decltype(reference)>>([&]<auto index>() {
			CHECK(std::get<index>(reference) != std::get<index>(r2));
			if constexpr (index == 3)
				CHECK(std::get<index>(reference) == std::get<index>(r2) + 2.5f);
			else
				CHECK(std::get<index>(reference) == std::get<index>(r2) + 1);
		});

		CHECK(std::get<2>(reference) == "abc1");
		CHECK(std::get<3>(reference) == (3.14f + 2.5f));
		CHECK(std::get<3>(reference) != 4.14f);
	}
}
