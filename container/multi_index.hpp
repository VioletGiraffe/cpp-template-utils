#pragma once

#include "container/multimap_helpers.hpp"
#include "utility/extra_type_traits.hpp"

#include <map>
#include <memory>
#include <set>

template <typename T, auto primaryKeyFieldPtr, auto secondaryKeyFieldPtr>
class MultiIndexSet {
public:
	using PrimaryKeyType = member_type_from_ptr_t<primaryKeyFieldPtr>;
	using SecondaryKeyType = member_type_from_ptr_t<secondaryKeyFieldPtr>;

	using value_type = T;

private:
	struct PrimaryKeyComparator
	{
		inline constexpr bool operator()(const T& left, const T& right) const noexcept
		{
			return left.*primaryKeyFieldPtr < right.*primaryKeyFieldPtr;
		}

		inline constexpr bool operator()(const PrimaryKeyType& key, const T& item) const noexcept
		{
			return key < item.*primaryKeyFieldPtr;
		}

		inline constexpr bool operator()(const T& item, const PrimaryKeyType& key) const noexcept
		{
			return item.*primaryKeyFieldPtr < key;
		}

		using is_transparent = void;
	};

	struct SecondaryKeyComparator {
		inline constexpr bool operator()(const T& left, const T& right) const noexcept {
			return left.*secondaryKeyFieldPtr < right.*secondaryKeyFieldPtr;
		}
	};

	std::set<T, PrimaryKeyComparator> _primarySet;
	std::multimap<SecondaryKeyType, T*, SecondaryKeyComparator> _secondaryIndex;

public:
	using iterator = multimap_value_iterator<T, typename decltype(_primarySet)::iterator>;

	template<class... Args>
	auto emplace(Args&&... args) {
		auto result = _primarySet.emplace(std::forward<Args>(args)...);
		if (result.second)
			_secondaryIndex.emplace(result.first->*secondaryKeyFieldPtr, std::addressof(*result.first));

		return result;
	}

	auto findPrimary(const PrimaryKeyType& key) const noexcept {
		return _primarySet.find(key);
	}

	std::pair<iterator, iterator> findSecondary(const SecondaryKeyType& key) const noexcept {
		const auto range = _secondaryIndex.equal_range(key);
		return { range.first, range.second };
	}

	// Returns a pair of iterators [begin, end) matching the range of keys [lowerBound, upperBound)
	// such that [begin, end) contains all the items for which lowerBound <= key < upperBound
	std::pair<iterator, iterator> findSecondaryInRange(const SecondaryKeyType& lowerBound, const SecondaryKeyType& upperBound) const noexcept {
		return { _secondaryIndex.lower_bound(lowerBound), _secondaryIndex.upper_bound(upperBound) };
	}

	size_t size() const noexcept {
		return _primarySet.size();
	}

	bool empty() const noexcept {
		return _primarySet.empty();
	}
};
