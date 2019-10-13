#pragma once
#include "template_magic.hpp"

#include <utility>

template <int First, int Last, typename Functor>
constexpr void static_for([[maybe_unused]] Functor&& f)
{
	if constexpr (First < Last)
	{
		f(value_as_type<First>{});
		static_for<First + 1, Last, Functor>(std::forward<Functor>(f));
	}
}
