#pragma once

#include "../parameter_pack/parameter_pack_helpers.hpp"

#include <array>
#include <tuple>
#include <type_traits>
#include <utility>

namespace tuple {
	namespace detail {
		template <class Tuple, class F, size_t I>
		constexpr void invoke(Tuple&& tuple, F f) noexcept
		{
			f(std::get<I>(tuple));
		}

		template <class Tuple, class F, size_t... I>
		[[nodiscard]] constexpr auto make_functor(std::index_sequence<I...>) noexcept
		{
			return std::array<void (*)(Tuple &, F), sizeof...(I)>{invoke<Tuple, F, I>...};
		}

		template <class Tuple, class F, size_t... I>
		[[nodiscard]] constexpr auto make_functor_const(std::index_sequence<I...>) noexcept
		{
			return std::array<void (*)(const Tuple &, F), sizeof...(I)>{invoke<const Tuple, F, I>...};
		}
	}

	template <class Tuple>
	inline constexpr size_t tuple_size_v_omnivorous = std::tuple_size_v<std::remove_reference_t<Tuple>>;

	template <class Tuple, class F>
	constexpr void visit(Tuple&& t, size_t index, F f) noexcept
	{
		if constexpr (std::is_const_v<Tuple>)
		{
			constexpr auto functors = detail::make_functor_const<Tuple, F>(std::make_index_sequence<tuple_size_v_omnivorous<Tuple>>());
			return functors[index](std::forward<Tuple>(t), f);
		}
		else
		{
			constexpr auto functors = detail::make_functor<Tuple, F>(std::make_index_sequence<tuple_size_v_omnivorous<Tuple>>());
			return functors[index](std::forward<Tuple>(t), f);
		}
	}

	template <typename T, typename... Args>
	[[nodiscard]] consteval size_t indexForType(const std::tuple<Args...>&) noexcept {
		return ::pack::index_for_type_v<T, Args...>;
	}

	template <class Tuple>
	[[nodiscard]] consteval size_t size(Tuple&&) noexcept
	{
		return tuple_size_v_omnivorous<Tuple>;
	}

	template<class Tuple, class F>
	void for_each(Tuple&& tuple, F func) noexcept {
		static_for<0, tuple_size_v_omnivorous<Tuple>>([&]<auto I>() {
			::tuple::visit(tuple, I, func);
		});
	}
}
