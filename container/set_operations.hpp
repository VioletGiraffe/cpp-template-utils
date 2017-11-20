#pragma once

#include "std_container_helpers.hpp"

#include <algorithm>
#include <functional>
#include <iterator>
#include <numeric>
#include <set>
#include <type_traits>

namespace SetOperations {

//
// For a container of containers, returns the range containing the longest starting sequence that's common for all the child sets (or empty range)
//


template <class OrderedSetType, template<class> class SupersetType>
OrderedSetType longestCommonStart(const SupersetType<OrderedSetType>& superset)
{
	if (superset.empty())
		return OrderedSetType();

	const size_t minSubsetLength = std::accumulate(cbegin_to_end(superset), std::numeric_limits<size_t>::max(), [](size_t currentMin, const auto& subset) {
		return std::min(currentMin, (size_t)subset.size());
	});

	size_t maxCommonSubsetLength = 0;
	for (maxCommonSubsetLength = 0; maxCommonSubsetLength < minSubsetLength; ++maxCommonSubsetLength)
	{
		auto item = *(superset.front().cbegin() + maxCommonSubsetLength);
		for (auto subset = superset.cbegin() + 1, end = superset.cend(); subset != end; ++subset)
		{
			if (item != *(subset->cbegin() + maxCommonSubsetLength))
			{
				OrderedSetType commonSubset;
				std::copy(subset->cbegin(), subset->cbegin() + maxCommonSubsetLength, std::back_inserter(commonSubset));
				return commonSubset;
			}
		}
	}

	// No premature exit happened -> all the subsets are equal
	return superset.front();
}

//
// Returns the container of the same type as the argument, containing only the unique elements from the argument.
// Preserves order (if allowed by the container type).
//


template <class ContainerType>
inline ContainerType uniqueElements(const ContainerType& c)
{
	ContainerType result;
	std::set<typename ContainerType::value_type> helperSet;
	std::copy_if(cbegin_to_end(c), std::back_inserter(result), [&helperSet](const typename ContainerType::value_type& element){
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

	std::set_difference(cbegin_to_end(c1), cbegin_to_end(c2), std::back_inserter(result), comp);

	return result;
}

template <class ContainerType>
struct Diff
{
	ContainerType common_elements;
	ContainerType elements_from_a_not_in_b;
	ContainerType elements_from_b_not_in_a;
};

template <class ContainerType1, class ContainerType2, class ResultContainerType = ContainerType2>
Diff<ResultContainerType> calculateDiff(const ContainerType1& a, const ContainerType2& b)
{
	Diff<ResultContainerType> diff;

	for (const auto& item_a: a)
	{
		if (container_aware_find(b, item_a) == std::end(b))
			add_item(diff.elements_from_a_not_in_b, item_a);
	}

	for (const auto& item_b : b)
	{
		if (container_aware_find(a, item_b) == std::end(a))
			add_item(diff.elements_from_b_not_in_a, item_b);
		else
			add_item(diff.common_elements, item_b);
	}

	return diff;
}

} // namespace
