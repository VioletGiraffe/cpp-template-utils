#pragma once

#include <algorithm>
#include <set>

#define begin_to_end(container) std::begin(container), std::end(container)
#define cbegin_to_end(container) std::cbegin(container), std::cend(container)

// std::set with a transparent comparator
template <typename value_type>
using transparent_set = std::set<value_type, std::less<>>;

template <class Container, typename ItemType>
void add_item(Container& container, ItemType&& item) requires requires { container.push_back(item); }
{
	container.push_back(std::forward<ItemType>(item));
}

template <class Container, typename ItemType>
void add_item(Container& container, ItemType&& item) requires requires { container.insert(item); }
{
	container.insert(item);
}

template <class Container, typename ItemType>
auto container_aware_find(const Container& c, ItemType&& item) noexcept requires requires { c.find(item); }
{
	return c.find(std::forward<ItemType>(item));
}

template <class Container, typename ItemType>
auto container_aware_find(const Container& c, ItemType&& item) noexcept
{
	return std::find(begin_to_end(c), std::forward<ItemType>(item));
}
