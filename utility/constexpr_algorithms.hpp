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

template <auto first, auto last, typename Functor, typename ValueType>
constexpr void constexpr_from_runtime_value(const ValueType value, [[maybe_unused]] Functor&& f) noexcept
{
	if (first == value)
	{
		f(value_as_type<static_cast<ValueType>(first)>{});
		return;
	}

	if constexpr (first != last)
		constexpr_from_runtime_value<first + 1, last, Functor>(value, std::forward<Functor>(f));
}
