#include "3rdparty/catch2/catch.hpp"

#include "utility/odd_sized_integer.hpp"
#include "math/math.hpp"

TEST_CASE("Construction from and conversion to regular integer")
{
	static constexpr size_t N = 3;

	{
		odd_sized_integer<N> oi {7};
		REQUIRE(oi.toUi64() == 7);
		REQUIRE(oi.toUi64() == uint64_t{7});
	}

	static constexpr uint64_t max = Math::pow2(3 * 8) - 1;
	{
		odd_sized_integer<N> oi {max};
		REQUIRE(oi.toUi64() == max);
	}
}

TEST_CASE("Copy construction")
{
	odd_sized_integer<5> oi {5};
	const auto oi2 = oi;

	REQUIRE(oi2 == oi);
	REQUIRE(oi2 == 5);
}

TEST_CASE("Move construction")
{
	odd_sized_integer<5> oi {5};
	const auto oi2 = std::move(oi);

	REQUIRE(oi2 == 5);
}

TEST_CASE("Assignemnt and move assignment")
{
	odd_sized_integer<5> oi {5};

	odd_sized_integer<5> oi2;
	oi2 = 3;
	CHECK(oi2 == 3);

	oi2 = oi;
	CHECK(oi2 == 5);

	odd_sized_integer<5> oi3 {500'000};
	oi = std::move(oi3);
	CHECK(oi == 500'000);
}

TEST_CASE("comparison operators")
{
	odd_sized_integer<7> o1 {7};
	odd_sized_integer<7> o2 {7'000'000'000ULL};
	odd_sized_integer<7> o3 {7'000'000'000ULL};

	CHECK(o1 != 0);
	CHECK(o1 == 7);
	CHECK(o1 != o2);
	CHECK(o2 == o3);
	CHECK(!(o2 != o3));

	CHECK(o2 > o1);
	CHECK(o2 >= o1);
	CHECK(o1 < o2);
	CHECK(o1 <= o2);
	CHECK(o2 <= o3);
	CHECK(o2 >= o3);
	CHECK(!(o2 > o3));
	CHECK(!(o2 < o3));
	CHECK(!(o2 != o3));

	CHECK(o1 != o2.toUi64());
	CHECK(o2 == o3.toUi64());
	CHECK(!(o2 != o3.toUi64()));

	CHECK(o2 > o1.toUi64());
	CHECK(o2 >= o1.toUi64());
	CHECK(o1 < o2.toUi64());
	CHECK(o1 <= o2.toUi64());
	CHECK(o2 <= o3.toUi64());
	CHECK(o2 >= o3.toUi64());
	CHECK(!(o2 > o3.toUi64()));
	CHECK(!(o2 < o3.toUi64()));
	CHECK(!(o2 != o3.toUi64()));
}

TEST_CASE("Arithmetics")
{
	odd_sized_integer<7> oi {7};
	CHECK(oi + 10 == 17);
	CHECK(oi - 5 == 2);
	CHECK(oi * 3 == 21);
	CHECK(oi / 2 == 3);
	CHECK(oi % 3 == 1);
}
