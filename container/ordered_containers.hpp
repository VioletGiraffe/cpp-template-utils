#pragma once

#include "std_container_helpers.hpp"

#include <algorithm>
#include <assert.h>
#include <utility>

template <class ContainerType, typename Comparator = std::less<> /* transparent comparator */>
class ordered_container : public ContainerType
{
public:
	template <typename T>
	typename ContainerType::iterator find(const T& value);

	template <typename T>
	std::pair<typename ContainerType::iterator, bool> insert_into_sorted(const T& value);

	void sort();

private:
	bool _sorted = false;
};

template <class ContainerType, typename Comparator> template <typename T>
typename typename ContainerType::iterator ordered_container<ContainerType, Comparator>::ordered_container::find(const T& value)
{
	assert(_sorted);

	const auto end_iterator = end();
	const auto it = std::lower_bound(begin(), end_iterator, value, Comparator());
	return it != end_iterator && Comparator()(value, *it) == false ? it : end_iterator;
}

template <class ContainerType, typename Comparator> template <typename T>
std::pair<typename ContainerType::iterator, bool> ordered_container<ContainerType, Comparator>::insert_into_sorted(const T& value)
{
	assert(_sorted);

	const auto end_iterator = end();
	auto it = std::lower_bound(begin(), end_iterator, value, Comparator());
	if (it == end_iterator || Comparator()(value, *it) == true) // Item not yet present in the container
	{
		insert(value, it);
		return std::make_pair(it, true);
	}
	else
		return std::make_pair(it, false); // Item already present in the container
}


template <class ContainerType, typename Comparator /*= std::less<> /* transparent comparator */>
void ordered_container<ContainerType, Comparator>::sort()
{
	std::sort(begin(), end(), Comparator());
	_sorted = true;
}
