#pragma once

#include "container/multimap_helpers.hpp"
#include "utility/extra_type_traits.hpp"

#include <assert.h>
#include <initializer_list>
#include <map>
#include <memory>
#include <set>

template <typename...>
struct CheckType;

template <typename T, auto primaryKeyFieldPtr, auto secondaryKeyFieldPtr>
class MultiIndexSet {
public:
	using PrimaryKeyType = member_type_from_ptr_t<primaryKeyFieldPtr>;
	using SecondaryKeyType = member_type_from_ptr_t<secondaryKeyFieldPtr>;

	using value_type = const T;

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

	std::set<T, PrimaryKeyComparator> _primarySet;
	std::multimap<SecondaryKeyType, value_type*> _secondaryIndex;

public:
	using secondary_key_iterator = multimap_value_iterator<typename decltype(_secondaryIndex)::const_iterator>;

	constexpr MultiIndexSet() = default;

	constexpr MultiIndexSet(std::initializer_list<T> list) noexcept {
		for (const auto& item: list)
			emplace(item);
	}

	template <typename... Args>
	auto emplace(Args&&... args) {
		const auto result = _primarySet.emplace(std::forward<Args>(args)...);
		if (result.second)
			_secondaryIndex.emplace((*(result.first)).*secondaryKeyFieldPtr, std::addressof(*(result.first)));

		assert(_primarySet.size() == _secondaryIndex.size());
		return result;
	}

	auto findPrimary(const PrimaryKeyType& key) const noexcept {
		return _primarySet.find(key);
	}

	std::pair<secondary_key_iterator, secondary_key_iterator> findSecondary(const SecondaryKeyType& key) const noexcept {
		const auto range = _secondaryIndex.equal_range(key);
		return {range.first, range.second};
	}

	// Returns a pair of iterators [begin, end) matching the range of keys [lowerBound, upperBound)
	// such that [begin, end) contains all the items for which lowerBound <= key < upperBound
	std::pair<secondary_key_iterator, secondary_key_iterator> findSecondaryInRange(const SecondaryKeyType& lowerBound, const SecondaryKeyType& upperBound) const noexcept {
		assert(lowerBound <= upperBound);
		return { _secondaryIndex.lower_bound(lowerBound), _secondaryIndex.upper_bound(upperBound) };
	}

	void erase(typename decltype(_primarySet)::iterator it) noexcept
	{
		assert(it != _primarySet.end());
		const auto secondaryRange = findSecondary((*it).*secondaryKeyFieldPtr);
		assert(secondaryRange.first != secondaryRange.second);
		for (auto secondaryIt = secondaryRange.first; secondaryIt != secondaryRange.second;)
		{
			const auto current = secondaryIt;
			++secondaryIt;
			if (current->second == std::addressof(*it))
				_secondaryIndex.erase(current);
		}

		_primarySet.erase(it);

		assert(_primarySet.size() == _secondaryIndex.size());
	}

	size_t erase(const PrimaryKeyType& primaryKey) noexcept
	{
		const auto it = findPrimary(primaryKey);
		if (it == _primarySet.end())
			return 0;

		erase(it);
		return 1;
	}

	size_t erase(const T& item) noexcept
	{
		const auto it = _primarySet.find(item);
		if (it == _primarySet.end())
			return 0;

		erase(it);
		return 1;
	}

	size_t size() const noexcept {
		return _primarySet.size();
	}

	bool empty() const noexcept {
		return _primarySet.empty();
	}

	void clear() noexcept {
		_secondaryIndex.clear();
		_primarySet.clear();
	}

	auto begin() const {
		return _primarySet.begin();
	}

	auto end() const {
		return _primarySet.end();
	}
};
