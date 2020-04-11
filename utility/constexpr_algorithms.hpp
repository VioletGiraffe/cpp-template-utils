#pragma once
#include "template_magic.hpp"

#include <functional> // std::less
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
