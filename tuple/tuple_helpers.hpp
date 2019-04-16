#pragma once

#include "../parameter_pack/parameter_pack_helpers.hpp"

#include <array>
#include <tuple>
#include <type_traits>
#include <utility>

namespace tuple {
	template<class F, class... Ts, std::size_t... Is>
	void for_each(const std::tuple<Ts...>& tuple, F func, std::index_sequence<Is...>) {
		using expander = int[];
		(void)expander {
			0, ((void)func(std::get<Is>(tuple)), 0)...
		};
	}

	template<class F, class... Ts>
	void for_each(const std::tuple<Ts...>& tuple, F func) {
		::tuple::for_each(tuple, func, std::make_index_sequence<sizeof... (Ts)>());
	}

	template<class F, class... Ts, std::size_t... Is>
	void for_each(std::tuple<Ts...>& tuple, F func, std::index_sequence<Is...>) {
		using expander = int[];
		(void)expander {
			0, ((void)func(std::get<Is>(tuple)), 0)...
		};
	}

	template<class F, class... Ts>
	void for_each(std::tuple<Ts...>& tuple, F func) {
		::tuple::for_each(tuple, func, std::make_index_sequence<sizeof... (Ts)>());
	}
}

namespace tuple {
	template <typename T, typename... Args>
	constexpr size_t indexForType(const std::tuple<Args...>&) {
		return ::pack::index_for_type_v<T, Args...>;
	}
}

namespace tuple {
	namespace detail {
		template <class Tuple, class F, size_t I>
		void invoke(Tuple &tuple, F f)
		{
			f(std::get<I>(tuple));
		}

		template <class Tuple, class F, size_t... I>
		constexpr auto make_functor(std::index_sequence<I...>)
		{
			return std::array<void (*)(Tuple &, F), sizeof...(I)>{invoke<Tuple, F, I>...};
		}
	}

	template <class Tuple, class F>
	void visit(Tuple &t, size_t index, F f)
	{
		constexpr auto functors = detail::make_functor<Tuple, F>(std::make_index_sequence<std::tuple_size<Tuple>::value>());
		return functors[index](t, f);
	}
}
