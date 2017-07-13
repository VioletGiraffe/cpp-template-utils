#pragma once

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
	std::deque<typename ContainerType::value_type> common_elements;
	std::deque<typename ContainerType::value_type> elements_from_a_not_in_b;
	std::deque<typename ContainerType::value_type> elements_from_b_not_in_a;
};

template <class ContainerType1, class ContainerType2>
diff<ContainerType1> calculateDiff(const ContainerType1& a, const ContainerType2& b)
{
	diff<ContainerType1> diff;

	for (const auto& item_a: a)
	{
		if (std::find(std::begin(b), std::end(b), item_a) != std::end(b))
			diff.common_elements.push_back(item_a);
		else
			diff.elements_from_a_not_in_b.push_back(item_a);
	}

	for (const auto& item_b : b)
	{
		if (std::find(std::begin(a), std::end(a), item_b) == std::end(b))
			diff.elements_from_b_not_in_a.push_back(item_b);
	}

	return diff;
}

} // namespace
