#pragma once

#include <assert.h>
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
	namespace detail {
		// Index of the type in the tuple's list of types
		template<size_t N, typename A, typename...Fields>
		struct IndexForType;

		template<size_t N, typename A, typename...Fields>
		struct IndexForType<N, A, A, Fields...> {
			enum {value = N};
		};

		template<size_t N, typename A, typename B, typename...Fields>
		struct IndexForType<N, A, B, Fields...> {
			enum {value = IndexForType<N + 1, A, Fields...>::value};
		};
	}

	template <typename T, typename... Args>
	constexpr size_t indexForType(const std::tuple<Args...>&) {
		return detail::IndexForType<0, T, Args...>::value;
	}
}
