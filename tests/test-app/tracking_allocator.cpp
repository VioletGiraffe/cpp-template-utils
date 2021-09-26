#include "3rdparty/catch2/catch.hpp"
#include "container/tracking_allocator.hpp"

#include <map>
#include <vector>

TEST_CASE("Allocator, std::map", "[allocator]") {
	std::map<int, int, std::less<int>, TrackingAllocator<std::pair<const int, int>>> m;
	CHECK(m.get_allocator().bytes() == 40);

	m[2] = 0;
	size_t b = m.get_allocator().bytes();
	CHECK(b == 80);
	m.insert(std::pair{ 5, 1 });
	b = m.get_allocator().bytes();
	CHECK(b == 120);
	m.insert(std::pair{0, 2});
	b = m.get_allocator().bytes();
	CHECK(b == 160);

	auto m2 = m;
	m.insert(std::pair{ 3, 3 });
	b = m.get_allocator().bytes();
	CHECK(b == 200);
	b = m2.get_allocator().bytes();
	CHECK(b == 160);

	decltype(m) m3, m4;

	m3 = m2;
	b = m3.get_allocator().bytes();
	CHECK(b == 160);
	b = m.get_allocator().bytes();
	CHECK(b == 200);

	m4 = m;
	b = m4.get_allocator().bytes();
	CHECK(b == 200);
	m4 = m2;
	b = m4.get_allocator().bytes();
	CHECK(b == 160);

	m.emplace(-1, -1);
	b = m.get_allocator().bytes();
	CHECK(b == 240);
	m.erase(3);
	b = m.get_allocator().bytes();
	CHECK(b == 200);
}

TEST_CASE("Allocator, std::vector", "[allocator]") {
	std::vector<int, TrackingAllocator<int>> m;
	CHECK(m.get_allocator().bytes() == 0);

	m.reserve(3);
	size_t b = m.get_allocator().bytes();
	CHECK(b == 12);
	m.push_back(5);
	b = m.get_allocator().bytes();
	CHECK(b == 12);
	m.push_back(5);
	m.push_back(5);
	b = m.get_allocator().bytes();
	CHECK(b == 12);

	auto m2 = m;
	m.push_back(6);
	b = m.get_allocator().bytes();
	CHECK((b >= 16 && b <= 24));
	b = m2.get_allocator().bytes();
	CHECK(b == 12);

	decltype(m) m3, m4;

	m3 = m2;
	b = m3.get_allocator().bytes();
	CHECK(b == 12);
	b = m.get_allocator().bytes();
	CHECK((b >= 16 && b <= 24));

	m4 = m;
	b = m4.get_allocator().bytes();
	CHECK(b == 16);
	m4 = m2;
	b = m4.get_allocator().bytes();
	CHECK(b == 16);
	m4.shrink_to_fit();
	b = m4.get_allocator().bytes();
	//CHECK(b >= 12 && b <= 16);

	m.emplace_back(10);
	b = m.get_allocator().bytes();
	CHECK(b >= 20);
	m.shrink_to_fit();
	b = m.get_allocator().bytes();
	CHECK(b == 20);
	m.erase(m.begin());
	b = m.get_allocator().bytes();
	CHECK(b == 20);
	m.clear();
	b = m.get_allocator().bytes();
	CHECK(b == 20);
	m.push_back(-1);
	b = m.get_allocator().bytes();
	CHECK(b == 20);
	m.shrink_to_fit();
	b = m.get_allocator().bytes();
	CHECK(b == 4);
}
