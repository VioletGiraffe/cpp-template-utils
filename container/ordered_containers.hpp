#pragma once

#include "std_container_helpers.hpp"

#include <algorithm>
#include <assert.h>

template <class ContainerType, typename Comparator = std::less<> /* transparent (heterogenous) comparator */>
class ordered_container : public ContainerType
{
public:
	template <typename T>
	typename ContainerType::iterator find(const T& value);

	void sort();

private:
	bool _sorted = false;
};

template <class ContainerType, typename Comparator> template <typename T>
typename ContainerType::iterator ordered_container<ContainerType, Comparator>::ordered_container::find(const T& value)
{
	assert(_sorted);

	const auto end_iterator = ContainerType::end();
	const auto it = std::lower_bound(ContainerType::begin(), end_iterator, value, Comparator());
	return it != end_iterator && Comparator()(value, *it) == false ? it : end_iterator;
}


template <class ContainerType, typename Comparator /*= std::less<> */>
void ordered_container<ContainerType, Comparator>::sort()
{
	std::sort(ContainerType::begin(), ContainerType::end(), Comparator());
	_sorted = true;
}
