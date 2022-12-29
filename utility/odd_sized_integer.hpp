#pragma once

#include <compare>
#include <concepts>
#include <cstddef>

#include <stddef.h>
#include <stdint.h>
#include <string.h> // memcpy

template <size_t nBytes>
class odd_sized_integer
{
	static_assert(nBytes <= 8);
public:
	constexpr odd_sized_integer() noexcept = default;
	constexpr odd_sized_integer(uint64_t value)
	{
		::memcpy(_data, &value, nBytes);
	}

	[[nodiscard]] constexpr uint64_t toUi64() const noexcept
	{
		uint64_t value = 0;
		::memcpy(&value, _data, nBytes);
		return value;
	}

	[[nodiscard]] constexpr operator uint64_t() const noexcept
	{
		return toUi64();
	}

	[[nodiscard]] constexpr std::strong_ordering operator<=>(const odd_sized_integer& other) const noexcept
	{
		return toUi64() <=> other.toUi64();
	}

	template <std::integral I>
	[[nodiscard]] constexpr std::strong_ordering operator<=>(I other) const noexcept
	{
		return toUi64() <=> other;
	}

private:
	std::byte _data[nBytes];
};
