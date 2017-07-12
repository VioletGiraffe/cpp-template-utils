#pragma once

#include "iterator_helpers.hpp"

#include <algorithm>
#include <deque>
#include <functional>
#include <iterator>
#include <set>
#include <type_traits>

namespace SetOperations {

//
// Returns the container of the same type as the argument, containing only the unique elements from the argument.
// Preserves order (if allowed by the container type).
//


template <class ContainerType>
inline ContainerType uniqueElements(const ContainerType& c)
{
	ContainerType result;
	std::set<typename ContainerType::value_type> helperSet;
	std::copy_if(c.begin(), c.end(), std::back_inserter(result), [&helperSet](const typename ContainerType::value_type& element){
		return helperSet.insert(element).second; // true if a new element was inserted
	});

	return result;
}

template <typename ItemType>
inline const std::set<ItemType>& uniqueElements(const std::set<ItemType>& set)
{
	return set;
}

template <typename KeyType, typename ValueType>
inline const std::map<KeyType, ValueType>& uniqueElements(const std::map<KeyType, ValueType>& map)
{
	return map;
}

template <template<typename...> class OutputContainerType, class ContainerType1, class ContainerType2>
OutputContainerType<typename ContainerType1::value_type, std::allocator<typename ContainerType1::value_type>>
setTheoreticDifference(
	ContainerType1 c1,
	ContainerType2 c2,
	const std::function<bool(const typename ContainerType1::value_type&, const typename ContainerType1::value_type&)>& comp = [](const typename ContainerType1::value_type& l, const typename ContainerType1::value_type& r) {return l < r;})
{
	OutputContainerType<typename ContainerType1::value_type, std::allocator<typename ContainerType1::value_type>> result;
	std::sort(c1.begin(), c1.end(), comp);
	std::sort(c2.begin(), c2.end(), comp);

	std::set_difference(c1.begin(), c1.end(), c2.begin(), c2.end(), std::back_inserter(result), comp);

	return result;
}

template <class ContainerType>
struct diff
{
	std::deque<const_forward_iterator_wrapper<ContainerType>> common_elements;
	std::deque<const_forward_iterator_wrapper<ContainerType>> elements_from_a_not_in_b;
	std::deque<const_forward_iterator_wrapper<ContainerType>> elements_from_b_not_in_a;
};

template <class ContainerType, class Comparator = std::less<typename ContainerType::value_type>>
diff<ContainerType> calculateDiff(const ContainerType& a, const ContainerType& b)
{
	diff<ContainerType> diff;

	for (auto a_iterator = a.cbegin(), a_end = a.cend(); a_iterator < a_end; ++a_iterator)
	{
		if (std::find(std::begin(b), std::end(b), *a_iterator) != std::end(b))
			diff.common_elements.emplace_back(a, a_iterator);
		else
			diff.elements_from_a_not_in_b.emplace_back(a, a_iterator);
	}

	for (auto b_iterator = b.cbegin(), b_end = b.cend(); b_iterator < b_end; ++b_iterator)
	{
		if (std::find(std::begin(a), std::end(a), *b_iterator) == std::end(b))
			diff.elements_from_b_not_in_a.emplace_back(b, b_iterator);
	}

	return diff;
}

} // namespace
