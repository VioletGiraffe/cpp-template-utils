#pragma once
#include "template_magic.hpp"

#include <utility>

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
	constexpr size_t N = sizeof(container) / sizeof(container[0]);

	for (size_t i = 0; i < N - 1; ++i)
	{
		for (size_t k = i + 1; k < N; ++k)
		{
			if (container[k] < container[i])
			{
				const auto tmp = container[i];
				container[i] = container[k];
				container[k] = tmp;
			}
		}
	}
}