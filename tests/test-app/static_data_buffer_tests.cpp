#include "3rdparty/catch2/catch.hpp"
#include "utility/static_data_buffer.hpp"

TEST_CASE("data_buffer", "[data_buffer]") {

	{
		static_data_buffer<1024> buf;
		CHECK(buf.size() == 0);
		CHECK(buf.begin() == buf.end());
		CHECK(buf.pos() == 0);

		buf.reserve(100);
		CHECK(buf.size() == 100);
		CHECK(buf.begin() != buf.end());
		CHECK(buf.pos() == 0);

		for (auto& byte : buf)
			byte = std::byte{ 'A' };

		CHECK(buf.size() == 100);
		CHECK(buf.pos() == 0);

		bool eq = true;
		for (size_t i = 0; i < buf.size(); ++i)
		{
			if (buf.data()[i] != std::byte{ 'A' })
			{
				eq = false;
				return;
			}
		}

		REQUIRE(eq);

		buf.seek(buf.size());
		CHECK(buf.size() == 100);
		CHECK(buf.pos() == 100);

		static constexpr const char* text = "1234567890";
		CHECK(buf.write(text, 10));
		CHECK(buf.size() == 110);
		CHECK(buf.pos() == 110);

		buf.seek(100);
		CHECK(buf.size() == 110);
		CHECK(buf.pos() == 100);

		char test[10];
		CHECK(buf.read(test, 10));
		CHECK(memcmp(test, text, 10) == 0);
	}
}
