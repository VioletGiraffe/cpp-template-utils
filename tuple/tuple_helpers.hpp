#pragma once

#include <tuple>

namespace detail {
	// A tag type based on size_t for indexing a tuple
	template<size_t> struct tuple_iterator_tag{};

	template <typename Functor, class Tuple, size_t pos>
	void apply(const Functor f, const Tuple& t, tuple_iterator_tag<pos> ) {
		f(std::get< std::tuple_size<Tuple>::value-pos >(t));
		apply(f, t, tuple_iterator_tag<pos-1>());
	}

	template <typename Functor, class Tuple>
	void apply(const Functor f, const Tuple& t, tuple_iterator_tag<1> ) {
		f(std::get<std::tuple_size<Tuple>::value-1>(t));
	}
}

namespace tuple {
	template <typename Functor, class... Args>
	void for_each(const Functor f, const std::tuple<Args...>& t) {
		detail::apply(f, t, detail::tuple_iterator_tag<sizeof...(Args)>());
	}
}