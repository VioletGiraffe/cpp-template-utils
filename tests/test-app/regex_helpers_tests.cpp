#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include "3rdparty/catch2/catch.hpp"
RESTORE_COMPILER_WARNINGS

#include "regex/regex_helpers.hpp"

#include <memory>
#include <regex>
#include <string>

namespace {

	// std::allocator with a distinct type: instantiates basic_string on something other than the defaults
	template <typename T>
	struct TestAllocator : std::allocator<T>
	{
		constexpr TestAllocator() noexcept = default;

		template <typename U>
		constexpr TestAllocator(const TestAllocator<U>&) noexcept {}
	};

	using TestString = std::basic_string<char, std::char_traits<char>, TestAllocator<char>>;

	[[nodiscard]] std::string toStdString(const TestString& str)
	{
		return std::string(str.cbegin(), str.cend());
	}
}

TEST_CASE("regex_replace - where the matches sit", "[regex_helpers]")
{
	const std::regex digits{ "[0-9]+" };
	const auto bracketed = [](const std::smatch& match) { return "[" + match.str() + "]"; };

	SECTION("nothing matches")
	{
		CHECK(regex_helpers::regex_replace(std::string{ "abc" }, digits, bracketed) == "abc");
	}

	SECTION("an empty subject")
	{
		CHECK(regex_helpers::regex_replace(std::string{}, digits, bracketed).empty());
	}

	SECTION("one match, surrounded")
	{
		CHECK(regex_helpers::regex_replace(std::string{ "a12b" }, digits, bracketed) == "a[12]b");
	}

	SECTION("a match at the start")
	{
		CHECK(regex_helpers::regex_replace(std::string{ "12ab" }, digits, bracketed) == "[12]ab");
	}

	SECTION("a match at the end")
	{
		CHECK(regex_helpers::regex_replace(std::string{ "ab12" }, digits, bracketed) == "ab[12]");
	}

	SECTION("the whole subject is one match")
	{
		CHECK(regex_helpers::regex_replace(std::string{ "12" }, digits, bracketed) == "[12]");
	}

	SECTION("several matches")
	{
		CHECK(regex_helpers::regex_replace(std::string{ "1a22b333" }, digits, bracketed) == "[1]a[22]b[333]");
	}

	SECTION("adjacent matches")
	{
		const std::regex singleDigit{ "[0-9]" };
		CHECK(regex_helpers::regex_replace(std::string{ "12" }, singleDigit, bracketed) == "[1][2]");
	}
}

TEST_CASE("regex_replace - zero-length matches", "[regex_helpers]")
{
	const std::regex wordBoundary{ "\\b" };
	const auto marker = [](const std::smatch&) { return std::string{ "|" }; };

#ifdef _LIBCPP_VERSION
	// libc++ does not report the word boundary at the end of the subject, so the closing marker is missing there
	CHECK(regex_helpers::regex_replace(std::string{ "hi there" }, wordBoundary, marker) == "|hi| |there");
#else
	CHECK(regex_helpers::regex_replace(std::string{ "hi there" }, wordBoundary, marker) == "|hi| |there|");
#endif

	CHECK(regex_helpers::regex_replace(std::string{ "  " }, wordBoundary, marker) == "  ");
}

TEST_CASE("regex_replace - the callback receives the whole match", "[regex_helpers]")
{
	const std::regex pair{ "([a-z])([0-9])" };
	const auto swapped = [](const std::smatch& match) { return match[2].str() + match[1].str(); };

	CHECK(regex_helpers::regex_replace(std::string{ "a1-b2" }, pair, swapped) == "1a-2b");
}

TEST_CASE("regex_replace - a string with a non-default allocator", "[regex_helpers]")
{
	const std::regex digits{ "[0-9]+" };
	const auto bracketed = [](const auto& match) { return "[" + match.str() + "]"; };

	const TestString subject{ "a12b" };
	CHECK(toStdString(regex_helpers::regex_replace(subject, digits, bracketed)) == "a[12]b");
}

TEST_CASE("replace_match - one capture group is replaced", "[regex_helpers]")
{
	const std::string subject{ "prefix key = value suffix" };
	const std::regex assignment{ "([a-z]+) = ([a-z]+)" };

	std::smatch match;
	REQUIRE(std::regex_search(subject, match, assignment));
	REQUIRE(match.str() == "key = value");

	// Only the match comes back, never the text around it
	CHECK(regex_helpers::replace_match(match, 1, std::string{ "name" }) == "name = value");
	CHECK(regex_helpers::replace_match(match, 2, std::string{ "42" }) == "key = 42");
	CHECK(regex_helpers::replace_match(match, 0, std::string{ "nothing" }) == "nothing");

	// An empty replacement removes the group
	CHECK(regex_helpers::replace_match(match, 1, std::string{}) == " = value");
}

TEST_CASE("replace_match - a group in the middle of the match", "[regex_helpers]")
{
	const std::string subject{ "abc" };
	const std::regex surrounded{ "a(b)c" };

	std::smatch match;
	REQUIRE(std::regex_search(subject, match, surrounded));

	CHECK(regex_helpers::replace_match(match, 1, std::string{ "X" }) == "aXc");
}

TEST_CASE("replace_match - a match over a character array", "[regex_helpers]")
{
	const std::regex surrounded{ "a(b)c" };

	std::cmatch match;
	REQUIRE(std::regex_search("abc", match, surrounded));

	CHECK(regex_helpers::replace_match(match, 1, std::string{ "X" }) == "aXc");
}
