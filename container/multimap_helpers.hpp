#pragma once

#include <iterator>
#include <memory>

template <typename ValueType, typename ActualMultimapIterator>
struct multimap_value_iterator {
	explicit multimap_value_iterator(const ActualMultimapIterator& iterator) -> multimap_value_iterator<decltype(typename std::iterator_traits<ActualMultimapIterator>::value_type::second), ActualMultimapIterator>
		: _iterator{iterator}
	{}

	multimap_value_iterator() = default;
	multimap_value_iterator(const multimap_value_iterator&) = default;
	multimap_value_iterator(multimap_value_iterator&&) = default;
	multimap_value_iterator& operator=(const multimap_value_iterator&) = default;
	multimap_value_iterator& operator=(multimap_value_iterator&&) = default;
	~multimap_value_iterator() = default;

	multimap_value_iterator& operator++() {
		++_iterator;
		return *this;
	}

	multimap_value_iterator& operator--() {
		--_iterator;
		return *this;
	}

	multimap_value_iterator operator++(int) {
		const auto old = *this;
		++*this;
		return old;
	}

	multimap_value_iterator operator--(int) {
		const auto old = *this;
		--*this;
		return old;
	}

	ValueType& operator*() const {
		return _iterator->second;
	}

	ValueType* operator->() const {
			return std::addressof(_iterator->second);
		}

private:
	ActualMultimapIterator _iterator;
};
