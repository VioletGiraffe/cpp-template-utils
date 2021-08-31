#include "3rdparty/catch2/catch.hpp"
#include "parameter_pack/parameter_pack_helpers.hpp"

#include <variant>

TEST_CASE("pack::type_by_index", "[pack]") {
	static_assert (std::is_same_v<pack::type_by_index<0, int>, int>);

	static_assert (std::is_same_v<pack::type_by_index<0, int, double, std::string, std::string&>, int>);
	static_assert (std::is_same_v<pack::type_by_index<1, int, double, std::string, std::string&>, double>);
	static_assert (std::is_same_v<pack::type_by_index<2, int, double, std::string, std::string&>, std::string>);
	static_assert (!std::is_same_v<pack::type_by_index<2, int, double, std::string, std::string&>, std::string&>);
	static_assert (std::is_same_v<pack::type_by_index<3, int, double, std::string, std::string&>, std::string&>);

	static_assert (std::is_same_v<pack::first_type<class S, int, double, std::string, std::string&>, class S>);

	CHECK(std::is_same_v<pack::type_by_index<3, int, double, std::string, std::string&>, std::string&>);
}

TEST_CASE("pack::index_for_type", "[pack]") {
	static_assert (pack::index_for_type<int, float, int, std::string, std::string&>() == 1);
	static_assert (pack::index_for_type<float, float, int, std::string, std::string&>() == 0);
	static_assert (pack::index_for_type<float, float, int, float, std::string, std::string&>() == 0); // Returning the first occurrence of the type
	static_assert (pack::index_for_type<std::string, float, int, std::string, std::string&>() == 2);
	static_assert (pack::index_for_type<std::string&, float, int, std::string, std::string&>() == 3);
	static_assert (!pack::index_for_type<const std::string&, float, int, std::string, std::string&>());
	static_assert (!pack::index_for_type<const std::string&>());
	static_assert (static_cast<bool>(pack::index_for_type<std::string&, float, int, std::string, std::string&>()));

	CHECK(pack::index_for_type_v<std::string&, float, int, std::string, std::string&> == 3);
}

TEST_CASE("pack::type_count", "[pack]") {
	static_assert (pack::type_count<int, float, int, std::string, std::string&>() == 1);
	static_assert (pack::type_count<std::string, float, std::string, int, std::string, std::string&>() == 2);
	static_assert (pack::type_count<std::string&, float, std::string, int, std::string, std::string&>() == 1);
	static_assert (pack::type_count<std::string&, float, std::string, int, std::string, std::string>() == 0);
	static_assert (pack::type_count<std::string&>() == 0);

	CHECK(pack::type_count<class S, float, std::string, int, std::string, std::string&, int, class S>() == 1);
}

TEST_CASE("pack::value_by_index at compile time", "[pack]") {
	CHECK(pack::value_by_index<0>('a') == 'a');
	CHECK(pack::value_by_index<0>(3.14f, 'a', -600) == 3.14f);
	CHECK(pack::value_by_index<1>(3.14f, 'a', -600) == 'a');
	CHECK(pack::value_by_index<2>(3.14f, 'a', -600) == -600);

	constexpr auto val = pack::value_by_index<0>(3.14f, 'a', -600);
	static_assert(val == 3.14f);
}

TEST_CASE("pack::for_value", "[pack]") {
	SECTION ("0 items") {
		std::vector<int> v;
		pack::for_value([&](auto&& value){
			v.push_back(value);
		});

		CHECK(v.empty());
	}

	SECTION ("1 item") {
		std::vector<int> v;
		pack::for_value([&](auto&& value){
			v.push_back(value);
		}, -116);

		CHECK(v.size() == 1);
		CHECK(v[0] == -116);
	}

	SECTION ("More items") {
		using VariantType = std::variant<float, std::string, int, double>;
		std::vector<VariantType> v;

		pack::for_value([&](auto&& value){
			v.emplace_back(value);
		}, -3.14f, "123", -116, std::numeric_limits<double>::min());

		CHECK(v.size() == 4);
		CHECK(v[0] == VariantType{-3.14f});
		CHECK(v[0] != VariantType{-3.15f});
		CHECK(v[1] == VariantType{std::string{"123"}});
		CHECK(v[2] == VariantType{-116});
		CHECK(v[3] == VariantType{std::numeric_limits<double>::min()});
		CHECK(v[3] != VariantType{0.0});
	}
}
