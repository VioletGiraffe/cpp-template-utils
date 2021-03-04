#pragma once

#include <vector>

template <typename T>
struct vector2D final : public std::vector<std::vector<T>>
{
	void resize(const size_t M, const size_t N)
	{
		std::vector<std::vector<T>>::resize(M);
		for (auto& row : *this)
		{
			row.resize(N);
		}
	}

	void fillRow(const size_t rowIndex, const T& value)
	{
		for (auto& item : (*this)[rowIndex])
			item = value;
	}

	[[nodiscard]] size_t width() const noexcept
	{
		return empty() ? 0 : this->front().size();
	}

	[[nodiscard]] size_t height() const noexcept
	{
		return size();
	}

private:
	[[nodiscard]] size_t size() const noexcept
	{
		return std::vector<std::vector<T>>::size();
	}

	[[nodiscard]] bool empty() const noexcept
	{
		return std::vector<std::vector<T>>::empty();
	}
};
