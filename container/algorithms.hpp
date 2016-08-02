#pragma once

#include <algorithm>

namespace ContainerAlgorithms {

//
// Erases all matching elements from the container
//

template <class ContainerType, typename ArgumentType>
inline void erase_all_occurrences(ContainerType& container, const ArgumentType& item)
{
	container.erase(std::remove(container.begin(), container.end(), item), container.end());
}

template <class ContainerType>
inline void erase_if(ContainerType& container, const std::function<bool(const typename ContainerType::value_type&)>& unaryPredicate)
{
	container.erase(std::remove_if(container.begin(), container.end(), unaryPredicate), container.end());
}

} // namespace
