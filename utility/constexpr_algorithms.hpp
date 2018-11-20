#pragma once

#include <type_traits>

template <int First, int Last, typename Functor>
constexpr void static_for(Functor&& f)
{
	if constexpr (First < Last)
	{
		f(std::integral_constant<int, First>{});
		static_for<First + 1, Last, Functor>(std::forward<Functor>(f));
	}
}
