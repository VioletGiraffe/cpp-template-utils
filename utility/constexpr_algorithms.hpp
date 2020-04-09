#pragma once
#include "template_magic.hpp"

#include <utility>
#include <vector> // std::size

template <int First, int Last, typename Functor>
constexpr void static_for([[maybe_unused]] Functor&& f) noexcept
{
	if constexpr (First < Last)
	{
		f(value_as_type<First>{});
		static_for<First + 1, Last, Functor>(std::forward<Functor>(f));
	}
}

template <int First, int Last, typename Functor>
constexpr void constexpr_for([[maybe_unused]] Functor&& f) noexcept
{
	static_for<First, Last>(std::forward<Functor>(f));
}

template <typename Container>
constexpr void constexpr_sort(Container& container) noexcept
{
	const size_t N = std::size(container);

	for (size_t i = 0; i < N - 1; ++i)
	{
		for (size_t k = i + 1; k < N; ++k)
		{
			if (container[k] < container[i])
				std::swap(container[i], container[k]);
		}
	}
}
