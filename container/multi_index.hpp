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
		inline constexpr bool operator()(const T& left, const T& right) const
		{
			return left.*primaryKeyFieldPtr < right.*primaryKeyFieldPtr;
		}

		inline constexpr bool operator()(const PrimaryKeyType& key, const T& item) const
		{
			return key < item.*primaryKeyFieldPtr;
		}

		inline constexpr bool operator()(const T& item, const PrimaryKeyType& key) const
		{
			return item.*primaryKeyFieldPtr < key;
		}

		using is_transparent = void;
	};

	static constexpr auto secondaryKeyComparator = [](const T& left, const T& right) {return left.*secondaryKeyFieldPtr < right.*secondaryKeyFieldPtr;};

	std::set<T, PrimaryKeyComparator> _primarySet;
	std::multimap<SecondaryKeyType, T*, decltype(secondaryKeyComparator)> _secondaryIndex;

public:
	using iterator = multimap_value_iterator<T, typename decltype(_primarySet)::iterator>;

	template<class... Args>
	auto emplace(Args&&... args) {
		auto result = _primarySet.emplace(std::forward<Args>(args)...);
		if (result.second)
			_secondaryIndex.emplace(result.first->*secondaryKeyFieldPtr, std::addressof(*result.first));

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
