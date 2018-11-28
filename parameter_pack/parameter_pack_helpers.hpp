#pragma once

#include <tuple>

namespace pack {
	namespace detail {
		// Index of the type in the list of types
		template<size_t N, typename A, typename...Fields>
		struct IndexForType;

		template<size_t N, typename A, typename...Fields>
		struct IndexForType<N, A, A, Fields...> {
			static constexpr size_t value = N;
		};

		template<size_t N, typename A, typename B, typename...Fields>
		struct IndexForType<N, A, B, Fields...> {
			static constexpr size_t value = IndexForType<N + 1, A, Fields...>::value;
		};
	}

	template <typename T, typename... Args>
	constexpr size_t index_for_type_v = detail::IndexForType<0, T, Args...>::value;

	template <size_t index, typename... Args>
	auto value_by_index(Args&&... args) noexcept {
		return std::get<index>(std::forward_as_tuple(std::forward<Args>(args)...));
	}

	template <typename Functor, class Arg, class... Args>
	void apply(Functor&& f, Arg&& a, Args&&... args) {
		f(std::forward<Arg>(a));
		if constexpr (sizeof...(args) > 0)
			apply(std::forward<Functor>(f), std::forward<Args>(args)...);
	}
}
