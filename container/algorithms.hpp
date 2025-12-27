#pragma once

#include <algorithm>
#include <utility>

namespace ContainerAlgorithms {

//
// Erases all matching elements from the container
//

template <class ContainerType, typename PredicateType>
inline void erase_if(ContainerType& container, PredicateType&& unaryPredicate)
{
	container.erase(std::remove_if(container.begin(), container.end(), std::forward<PredicateType>(unaryPredicate)), container.end());
}

} // namespace ContainerAlgorithms
