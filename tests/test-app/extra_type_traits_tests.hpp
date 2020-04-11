#pragma once

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
}
