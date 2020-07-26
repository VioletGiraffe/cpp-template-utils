#pragma once
#include "template_magic.hpp"

#include <type_traits>
#include <utility>

template <int First, int Last, typename Functor>
constexpr void static_for([[maybe_unused]] Functor&& f) noexcept
{
	if constexpr (First < Last)
	{
		if constexpr (std::is_same_v<bool, std::invoke_result_t<Functor()>>)
		{
			if (f(value_as_type<First>{}) == false)
				return;
		}
		else
			f(value_as_type<First>{});

		static_for<First + 1, Last, Functor>(std::forward<Functor>(f));
	}
}

template <int First, int Last, typename Functor>
constexpr void constexpr_for(Functor&& f) noexcept
{
	static_for<First, Last>(std::forward<Functor>(f));
}
