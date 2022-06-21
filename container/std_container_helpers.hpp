#pragma once

#include <algorithm>
#include <set>
#include <type_traits>

#define begin_to_end(container) std::begin(container), std::end(container)
#define cbegin_to_end(container) std::cbegin(container), std::cend(container)

template <typename value_type>
using transparent_set = std::set<value_type, std::less<>>;

namespace detail {

	using no = char;
	using yes = no(&)[2];

	no detect_push_back(...);
	no detect_find(...);

	// push_back checking utility
	template <class C>
	auto detect_push_back(C &c) -> typename std::conditional< false, decltype(c.push_back(std::declval< typename C::value_type >())), yes >::type;

	template <class C>
	struct has_push_back : std::integral_constant< bool, sizeof(detect_push_back(std::declval<C&>())) == sizeof(yes) > {};

	// find checking utility
	template <class C>
	auto detect_find(C &c) -> typename std::conditional< false, decltype(c.find(std::declval< typename C::value_type >())), yes >::type;

	template <class C>
	struct has_find : std::integral_constant< bool, sizeof(detect_find(std::declval<C&>())) == sizeof(yes) > {};

	template <class Container, typename ItemType>
	void add(Container &c, const ItemType& item, std::true_type)
	{
		c.push_back(item);
	}

	template <class Container, typename ItemType>
	void add(Container &c, const ItemType& item, std::false_type)
	{
		c.insert(item);
	}

	template <class Container, typename ItemType>
	auto find(Container &c, const ItemType& item, std::true_type)
	{
		return c.find(item);
	}

	template <class Container, typename ItemType>
	auto find(Container &c, const ItemType& item, std::false_type)
	{
		return std::find(begin_to_end(c), item);
	}

} // detail

// Calls 'push_back(item)' if available, otherwise 'insert(item)
template <class Container, typename ItemType>
void add_item(Container& container, const ItemType& item)
{
	add(container, item, detail::has_push_back<Container>());
}

// Calls 'container.find(item)' if available, otherwise 'std::find(container.begin(), container.end(), item)
template <class Container, typename ItemType>
auto container_aware_find(Container& container, const ItemType& item)
{
	return find(container, item, detail::has_find<Container>());
}
