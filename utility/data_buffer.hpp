#pragma once

#include <span>
#include <string>

class data_buffer
{
	std::basic_string<char8_t> _data;

public:
	using iterator = decltype(_data)::iterator;
	using const_iterator = decltype(_data)::const_iterator;
	using value_type = decltype(_data)::value_type;

	data_buffer() noexcept = default;
	data_buffer(size_t size) noexcept : _data(size, 0) {}
	data_buffer(const void* data, size_t size) noexcept : _data(reinterpret_cast<const value_type*>(data), size) {}

	[[nodiscard]] inline auto span() const noexcept
	{
		return std::span<const value_type>{data(), size()};
	}

	[[nodiscard]] inline operator std::span<const value_type>() const
	{
		return span();
	}

	[[nodiscard]] inline value_type* data() & noexcept
	{
		return _data.data();
	}

	[[nodiscard]] inline const value_type* data() const & noexcept
	{
		return _data.data();
	}

	[[nodiscard]] inline const value_type* data() const && noexcept
	{
		return _data.data();
	}

	[[nodiscard]] inline const value_type* constData() const noexcept
	{
		return data();
	}

	[[nodiscard]] inline auto operator[](const size_t i) const & noexcept
	{
		return _data[i];
	}

	[[nodiscard]] inline auto operator[](const size_t i) const && noexcept
	{
		return _data[i];
	}

	[[nodiscard]] inline auto& operator[](const size_t i) & noexcept
	{
		return _data[i];
	}

	[[nodiscard]] inline size_t size() const noexcept
	{
		return _data.size();
	}

	[[nodiscard]] inline auto begin() & noexcept
	{
		return _data.begin();
	}

	[[nodiscard]] inline auto begin() const & noexcept
	{
		return _data.begin();
	}

	[[nodiscard]] inline auto end() & noexcept
	{
		return _data.end();
	}

	[[nodiscard]] inline auto end() const & noexcept
	{
		return _data.end();
	}
};
