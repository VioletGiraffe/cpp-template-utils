#pragma once

#include "container/multimap_helpers.hpp"

#include <map>
#include <memory>
#include <set>

template <typename T, typename PrimaryKey, class PrimaryComparator, typename SecondaryKey, class SecondaryComparator, class SecondaryKeyGetter>
class MultiIndexSet {
	std::set<PrimaryKey, T, PrimaryComparator> _primarySet;
	std::multimap<SecondaryKey, T*, SecondaryComparator> _secondaryIndex;

public:
	using iterator = multimap_value_iterator<T, typename decltype(_primarySet)::iterator>;

	template<class... Args>
	auto emplace(Args&&... args) {
		auto result = _primarySet.emplace(std::forward<Args>(args)...);
		if (result.second)
			_secondaryIndex.emplace(SecondaryKeyGetter{}(*result.first), std::addressof(*result.first));

		return result;
	}

	auto findPrimary(const PrimaryKey& key) const {
		return _primarySet.find(key);
	}

	std::pair<iterator, iterator> findSecondary(const SecondaryKey& key) const {
		const auto range = _secondaryIndex.equal_range(key);
		return { range.first, range.second };
	}
};