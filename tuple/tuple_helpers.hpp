#pragma once

#include <assert.h>
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

	template <size_t I>
	struct visit_impl
	{
		template <typename T, typename F>
		static void visit(T& tup, size_t idx, F fun)
		{
			if (idx == I - 1) fun(std::get<I - 1>(tup));
			else visit_impl<I - 1>::visit(tup, idx, fun);
		}

		template <typename T, typename F>
		static void visit(const T& tup, size_t idx, F fun)
		{
			if (idx == I - 1) fun(std::get<I - 1>(tup));
			else visit_impl<I - 1>::visit(tup, idx, fun);
		}
	};

	template <>
	struct visit_impl<0>
	{
		template <typename T, typename F>
		static void visit(T& tup, size_t idx, F fun) { assert(false); }

		template <typename T, typename F>
		static void visit(const T& tup, size_t idx, F fun) { assert(false); }
	};

	template <typename F, typename... Ts>
	void visit_at(const std::tuple<Ts...>& tup, size_t idx, F fun)
	{
		visit_impl<sizeof...(Ts)>::visit(tup, idx, fun);
	}

	template <typename F, typename... Ts>
	void visit_at(std::tuple<Ts...>& tup, size_t idx, F fun)
	{
		visit_impl<sizeof...(Ts)>::visit(tup, idx, fun);
	}
}