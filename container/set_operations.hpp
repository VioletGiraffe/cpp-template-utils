#pragma once

#include "std_container_helpers.hpp"
#include "../utility/extra_type_traits.hpp"

#include <algorithm>
#include <iterator>
#include <map>
#include <numeric>
#include <set>
#include <type_traits>
#include <vector>

namespace SetOperations {

//
// For a container of containers, returns the range containing the longest starting sequence that's common for all the child sets (or empty range)
//

template <template <typename...> class SupersetType,
		  typename OrderedContainerType, typename ... OtherTypes>
OrderedContainerType longestCommonStart(SupersetType<OrderedContainerType, OtherTypes...> const & superset)
{
	if (superset.empty())
		return OrderedContainerType();

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
				OrderedContainerType commonSubset;
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

enum class ItemOrder {
	DontPreserveOrder,
	KeepLastOccurrence,
	KeepFirstOccurrence
};

namespace detail {
	template<typename T>
	concept HasReserve = requires(T t) {
		{ t.reserve() } -> std::same_as<void>;
	};
}

template <ItemOrder order = ItemOrder::DontPreserveOrder, class ContainerType>
[[nodiscard]] ContainerType uniqueElements(const ContainerType& c)
{
	using ConstIterator = typename ContainerType::const_iterator;

	struct ItemRef {
		ConstIterator it;

		inline bool operator<(const ItemRef& other) const {
			return *it < *other.it;
		}
	};

	ContainerType result;

	std::set<ItemRef> helperSet;
	// Inserting into std::set to check whether or not the item is unique.
	for (auto it = c.cbegin(), end = c.cend(); it != end; ++it)
	{
		ItemRef ref{ it };
		ref.it = it;
		auto insertionResult = helperSet.emplace(ItemRef{it}); // true if a new element was inserted
		const bool unique = insertionResult.second;
		if constexpr (order == ItemOrder::DontPreserveOrder)
		{
			if (unique)
				result.insert(result.end(), *it);
		}
		else if constexpr (order == ItemOrder::KeepFirstOccurrence)
		{
		}
		else if constexpr (order == ItemOrder::KeepLastOccurrence)
		{
			if (!unique)
			{
				helperSet.erase(insertionResult.first);
				helperSet.insert(ItemRef{ it });
			}
		}
	}

	if constexpr (order != ItemOrder::DontPreserveOrder)
	{
		std::vector<ConstIterator> uniqueIterators;
		uniqueIterators.reserve(helperSet.size());
		for (const auto& itemRef: helperSet)
			uniqueIterators.emplace_back(itemRef.it);

		std::sort(uniqueIterators.begin(), uniqueIterators.end(), [](const ConstIterator& l, const ConstIterator& r){return l < r;});
		if constexpr (detail::HasReserve<ContainerType>)
			result.reserve(uniqueIterators.size());

		for (const auto it: uniqueIterators)
			result.insert(result.end(), *it);
	}

	return result;
}

template <typename ItemType>
[[nodiscard]] const std::set<ItemType>& uniqueElements(const std::set<ItemType>& set)
{
	return set;
}

template <typename KeyType, typename ValueType>
[[nodiscard]] const std::map<KeyType, ValueType>& uniqueElements(const std::map<KeyType, ValueType>& map)
{
	return map;
}

template <template<typename...> class OutputContainerType, class ContainerType1, class ContainerType2, typename ComparatorType = std::less<>>
[[nodiscard]] OutputContainerType<typename ContainerType1::value_type, std::allocator<typename ContainerType1::value_type>>
 setTheoreticDifference(
	ContainerType1 c1,
	ContainerType2 c2,
	ComparatorType comp = ComparatorType{})
{
	OutputContainerType<typename ContainerType1::value_type, std::allocator<typename ContainerType1::value_type>> result;
	std::sort(c1.begin(), c1.end(), comp);
	std::sort(c2.begin(), c2.end(), comp);

	std::set_difference(cbegin_to_end(c1), cbegin_to_end(c2), std::inserter(result, result.end()), comp);

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
[[nodiscard]] Diff<ResultContainerType> calculateDiff(const ContainerType1& a, const ContainerType2& b)
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

template <template<typename...> class C1, template<typename...> class C2, typename... T1s, typename... T2s, typename SortComp = std::less<>, typename EqualComp = std::equal_to<>>
[[nodiscard]] bool is_equal_sets(C1<T1s...>& c1, C2<T2s...>& c2, [[maybe_unused]] SortComp&& sortComp = {}, EqualComp&& eqComp = {}) noexcept
{
	if (c1.size() != c2.size())
		return false;

	if constexpr (!std::is_same_v<C1<int>, std::set<int>>)
		std::sort(begin_to_end(c1), sortComp);

	if constexpr (!std::is_same_v<C2<int>, std::set<int>>)
		std::sort(begin_to_end(c2), sortComp);

	return std::equal(cbegin_to_end(c1), cbegin_to_end(c2), eqComp);
}

} // namespace
