#pragma once

#include "../utility/constexpr_algorithms.hpp"
#include "../utility/template_magic.hpp"

#include <optional>
#include <tuple>
#include <type_traits>

namespace pack {

	template <size_t index, typename... Args>
	using type_by_index = std::tuple_element_t<index, std::tuple<Args...>>;

	template <typename T, typename... Args>
	[[nodiscard]] constexpr std::optional<size_t> index_for_type() noexcept
	{
		std::optional<size_t> index;
		static_for<0, sizeof...(Args)>([&index](auto i) {
			if constexpr (std::is_same_v<T, type_by_index<static_cast<size_t>(decltype(i){}), Args... >>)
				index = static_cast<size_t>(decltype(i){});
		});

		return index;
	}

    namespace detail {
    template <typename T, typename... Args>
    [[nodiscard]] constexpr size_t index_for_type_strict() noexcept
    {
        constexpr auto index = index_for_type<T, Args...>();
        static_assert (index, "Type not found in pack");
        return *index;
    }
    }

    template <typename T, typename... Args>
    constexpr size_t index_for_type_v = detail::index_for_type_strict<T, Args...>();

	template <typename T, typename... Args>
	constexpr bool has_type_v = index_for_type<T, Args...>();

	template <typename T, typename... Args>
	[[nodiscard]] constexpr size_t type_count() noexcept
	{
		size_t count = 0;
		static_for<0, sizeof...(Args)>([&count](auto i) {
			if constexpr (std::is_same_v < T, type_by_index < static_cast<size_t>(decltype(i){}), Args... >> )
				++count;
		});

		return count;
	}

	template <typename... Args>
	using first_type = type_by_index<0, Args...>;

	template <size_t index, typename... Args>
	[[nodiscard]] constexpr auto value_by_index(Args&&... args) noexcept {
		return std::get<index>(std::forward_as_tuple(std::forward<Args>(args)...));
	}

	template <typename Functor, class Arg, class... Args>
	constexpr void apply(Functor&& f, Arg&& a, Args&&... args) noexcept {
		f(std::forward<Arg>(a));
		if constexpr (sizeof...(args) > 0)
			apply(std::forward<Functor>(f), std::forward<Args>(args)...);
	}

	template <typename... Args, typename Functor>
	constexpr void for_type(Functor&& f) noexcept {
		static_for<0, sizeof...(Args)>([f{ std::forward<Functor>(f) }](auto i) {
			f(type_wrapper<type_by_index<decltype(i)::value, Args...>>{});
		});
	}
}
