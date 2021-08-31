#include "3rdparty/catch2/catch.hpp"
#include "utility/extra_type_traits.hpp"

TEST_CASE("member_type_from_ptr_t", "[extra_type_traits]") {
	struct S {
		int intValue;
		float floatValue;
	};

	static_assert (std::is_same_v<member_type_from_ptr_t<&S::intValue>, int>);
	static_assert (std::is_same_v<member_type_from_ptr_t<&S::floatValue>, float>);
	static_assert (!std::is_same_v<member_type_from_ptr_t<&S::intValue>, float>);

	REQUIRE(true);
}

TEST_CASE("is_equal_comparable", "[extra_type_traits]") {
	struct non_comparable {
	};

	struct comparable {
		bool operator==(const comparable&) const { return true; }
		bool operator==(int) const { return true; }
	};

	static_assert (is_equal_comparable_v<int, int>);
	static_assert (!is_equal_comparable_v<int, int*>);
	static_assert (is_equal_comparable_v<comparable, comparable>);
	static_assert (!is_equal_comparable_v<non_comparable, non_comparable>);
	static_assert (is_equal_comparable_v<comparable, int>);
#if __cplusplus > 201703L && (!defined __GNUC__ || __GNUC__ >= 10)
	static_assert (is_equal_comparable_v<int, comparable>);
#else
	static_assert (!is_equal_comparable_v<int, comparable>);
#endif
	static_assert (!is_equal_comparable_v<comparable, non_comparable>);

	REQUIRE(true);
}
