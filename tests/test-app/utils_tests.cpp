#include "3rdparty/catch2/catch.hpp"
#include "utility/data_buffer.hpp"

#include <string.h>

TEST_CASE("data_buffer", "[data_buffer]") {

	{
		data_buffer buf;
		CHECK(buf.size() == 0);
		CHECK(buf.begin() == buf.end());
		CHECK(buf.span().size_bytes() == 0);
		CHECK(buf.span().empty());
	}

	{
		const char hello[] = {0, 'H', 'e', 0, 0, 'l', 'l', 'o'};
		data_buffer buf{hello, std::size(hello)};
		REQUIRE(std::equal(buf.begin(), buf.end(), std::begin(hello), std::end(hello)));
		REQUIRE(std::equal(buf.span().begin(), buf.span().end(), std::begin(hello), std::end(hello)));
		REQUIRE(buf.size() == buf.span().size_bytes());
		REQUIRE(buf.size() == buf.span().size());
		REQUIRE(buf.size() == std::size(hello));
		REQUIRE(::memcmp(buf.data(), hello, std::size(hello)) == 0);
		for (size_t i = 0; i < std::size(hello); ++i)
			REQUIRE(buf[i] == hello[i]);
	}

	{
		constexpr char hello[] = {0, 'H', 'e', 0, 0, 'l', 'l', 'o'};
		data_buffer buf{std::size(hello)};
		::memcpy(buf.data(), hello, buf.size());
		REQUIRE(std::equal(buf.begin(), buf.end(), std::begin(hello), std::end(hello)));
		REQUIRE(std::equal(buf.span().begin(), buf.span().end(), std::begin(hello), std::end(hello)));
		REQUIRE(buf.size() == buf.span().size_bytes());
		REQUIRE(buf.size() == buf.span().size());
		REQUIRE(buf.size() == std::size(hello));
		REQUIRE(::memcmp(buf.constData(), hello, std::size(hello)) == 0);
		for (size_t i = 0; i < std::size(hello); ++i)
			REQUIRE(buf[i] == hello[i]);
	}

	{
		constexpr char hello[] = {0, 'H', 'e', 0, 0, 'l', 'l', 'o'};
		data_buffer buf{std::size(hello)};
		for (size_t i = 0; i < std::size(hello); ++i)
			buf[i] = hello[i];

		REQUIRE(std::equal(buf.begin(), buf.end(), std::begin(hello), std::end(hello)));
		REQUIRE(std::equal(buf.span().begin(), buf.span().end(), std::begin(hello), std::end(hello)));
		REQUIRE(buf.size() == buf.span().size_bytes());
		REQUIRE(buf.size() == buf.span().size());
		REQUIRE(buf.size() == std::size(hello));
		REQUIRE(::memcmp(buf.constData(), hello, std::size(hello)) == 0);
		for (size_t i = 0; i < std::size(hello); ++i)
			REQUIRE(buf[i] == hello[i]);
	}
}
