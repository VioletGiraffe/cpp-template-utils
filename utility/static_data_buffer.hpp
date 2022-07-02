#pragma once

#include <array>
#include <string.h> // memcpy

template <size_t MaxSize>
class static_data_buffer
{
	std::array<std::byte, MaxSize> _data;
	size_t _actualSize = 0;
	size_t _pos = 0;

public:
	using value_type = typename decltype(_data)::value_type;

	static_assert(sizeof(value_type) == 1);

	constexpr bool reserve(const size_t nBytes) & noexcept
	{
		if (nBytes > MaxSize) [[unlikely]]
			return false;

		_actualSize = nBytes;
		return true;
	}

	constexpr bool read(void* dest, const size_t n) noexcept
	{
		if (_pos + n > _actualSize) [[unlikely]]
			return false;

		::memcpy(dest, _data.data() + _pos, n);
		_pos += n;
		return true;
	}

	constexpr bool write(const void* src, const size_t n) & noexcept
	{
		if (_pos + n > MaxSize) [[unlikely]]
			return false;

		::memcpy(_data.data() + _pos, src, n);
		_pos += n;

		if (_actualSize <= _pos)
			_actualSize = _pos;

		return true;
	}

	[[nodiscard]] constexpr size_t size() const noexcept
	{
		return _actualSize;
	}

	[[nodiscard]] constexpr size_t pos() const noexcept
	{
		return _pos;
	}

	constexpr void seek(const size_t pos) & noexcept
	{
		_pos = pos;
	}

	[[nodiscard]] constexpr const value_type* data() const & noexcept
	{
		return _data.data();
	}

	[[nodiscard]] constexpr auto begin() & noexcept
	{
		return _data.begin();
	}

	[[nodiscard]] constexpr auto begin() const & noexcept
	{
		return _data.begin();
	}

	[[nodiscard]] constexpr auto end() & noexcept
	{
		return begin() + _actualSize;
	}

	[[nodiscard]] constexpr auto end() const& noexcept
	{
		return begin() + _actualSize;
	}
};
