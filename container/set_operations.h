#pragma once

#include <algorithm>
#include <iterator>
#include <functional>
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

} // namespace
