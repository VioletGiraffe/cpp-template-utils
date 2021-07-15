#pragma once

#include "../utility/constexpr_algorithms.hpp"
#include "../utility/integer_literals.hpp"
#include "../utility/optional_consteval.hpp"
#include "../utility/template_magic.hpp"

#include <tuple>
#include <type_traits>

namespace pack {

	template <size_t index, typename... Args>
	using type_by_index = std::tuple_element_t<index, std::tuple<Args...>>;

	template <typename T, typename... Args>
	[[nodiscard]] consteval optional_consteval<size_t> index_for_type() noexcept
	{
		optional_consteval<size_t> index;
		consteval_for_z<0, sizeof...(Args)>([&index](auto i) consteval {
			if (!index) // The index of the first occurrence is stored
			{
				if constexpr (std::is_same_v<T, type_by_index<static_cast<size_t>(i), Args... >>)
					index = static_cast<size_t>(i);
			}
		});

		return index;
	}

	namespace detail {
		template <typename T, typename... Args>
		[[nodiscard]] consteval size_t index_for_type_strict() noexcept
		{
			constexpr auto index = index_for_type<T, Args...>();
			static_assert (index, "Type not found in pack");
			return *index;
		}
	}

	template <typename T, typename... Args>
	constexpr size_t index_for_type_v = detail::index_for_type_strict<T, Args...>();

	template <typename T, typename... Args>
	constexpr bool has_type_v = static_cast<bool>(index_for_type<T, Args...>());

	template <typename T, typename... Args>
	[[nodiscard]] consteval size_t type_count() noexcept
	{
		size_t count = 0;
		static_for<0, sizeof...(Args)>([&count](auto i) {
			if constexpr (std::is_same_v<T, type_by_index < static_cast<size_t>(decltype(i){}), Args... >> )
				++count;
			});

		return count;
	}

	template <typename... Args>
	using first_type = type_by_index<0, Args...>;

	template <size_t index, typename... Args>
	[[nodiscard]] consteval auto value_by_index(Args&&... args) noexcept {
		return std::get<index>(std::forward_as_tuple(std::forward<Args>(args)...));
	}

	template <typename Functor>
	constexpr void for_value(Functor&&) noexcept {
	}

	template <typename Functor, class Arg, class... Args>
	constexpr void for_value(Functor&& f, Arg&& a, Args&&... args) noexcept {
		f(std::forward<Arg>(a));
		for_value(std::forward<Functor>(f), std::forward<Args>(args)...);
	}

	template <typename... Args, typename Functor>
	consteval void for_type(Functor&& f) noexcept {
		static_for<0, sizeof...(Args)>([f{ std::forward<Functor>(f) }](auto i) {
			f(type_wrapper<type_by_index<decltype(i)::value, Args...>>{});
		});
	}
}

template <typename... Pack>
struct type_pack
{
	static constexpr size_t type_count = sizeof...(Pack);

	template <auto index>
	using Type = pack::type_by_index<index, Pack...>;

	template <typename T>
	static constexpr auto index_of = pack::detail::index_for_type_strict<T, Pack...>();

	template <typename... NewArgs>
	using Append = type_pack<Pack..., NewArgs...>;

	template <template <typename...> class T>
	using Construct = T<Pack...>;

	using Tuple = std::tuple<Pack...>;
};
