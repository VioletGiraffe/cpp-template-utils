#pragma once
#include "template_magic.hpp"

#include <type_traits>
#include <utility>

namespace detail {

	template <typename T, T First, T Last, typename Functor>
	constexpr void constexpr_for_T([[maybe_unused]] Functor&& f) noexcept
	{
		if constexpr (First < Last)
		{
			using I = value_as_type<First>;
			if constexpr (std::is_same_v<bool, std::invoke_result_t<Functor, I>>)
			{
				if (f(I{}) == false)
					return;
			}
			else
				f(I{});

			constexpr_for_T<T, First + 1, Last>(std::forward<Functor>(f));
		}
	}

	template <typename T, T First, T Last, typename Functor>
	consteval void consteval_for_T([[maybe_unused]] Functor&& f) noexcept
	{
		if constexpr (First < Last)
		{
			using I = value_as_type<First>;
			if constexpr (std::is_same_v<bool, std::invoke_result_t<Functor, I>>)
			{
				if constexpr (f(I{}) == false)
					return;
			}
			else
				f(I{});

			consteval_for_T<T, First + 1, Last>(std::forward<Functor>(f));
		}
	}
}

template <size_t First, size_t Last, typename Functor>
constexpr void constexpr_for_z([[maybe_unused]] Functor&& f) noexcept
{
	detail::constexpr_for_T<size_t, First, Last>(std::forward<Functor>(f));
}

template <int First, int Last, typename Functor>
constexpr void constexpr_for_i([[maybe_unused]] Functor&& f) noexcept
{
	detail::constexpr_for_T<int, First, Last>(std::forward<Functor>(f));
}

template <size_t First, size_t Last, typename Functor>
consteval void consteval_for_z([[maybe_unused]] Functor&& f) noexcept
{
	detail::consteval_for_T<size_t, First, Last>(std::forward<Functor>(f));
}

namespace detail {
	template <std::size_t First, std::size_t... Indices, typename Functor>
	constexpr void constexpr_for_fold_impl([[maybe_unused]] Functor&& f, std::index_sequence<Indices...>) noexcept
	{
		(std::forward<Functor>(f).template operator() < First + Indices > (), ...);
	}
}

template <std::size_t First, std::size_t Last, typename Functor>
constexpr void constexpr_for_fold([[maybe_unused]] Functor&& f) noexcept
{
	detail::constexpr_for_fold_impl<First>(std::forward<Functor>(f), std::make_index_sequence<Last - First>{});
}

#define static_for constexpr_for_z

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
