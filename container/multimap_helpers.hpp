#pragma once

#include <iterator>
#include <memory>

template <typename ValueType, typename ActualMultimapIterator>
struct multimap_value_iterator {
	explicit multimap_value_iterator(const ActualMultimapIterator& iterator) noexcept
		: _iterator{iterator}
	{}

	constexpr multimap_value_iterator() noexcept = default;
	constexpr multimap_value_iterator(const multimap_value_iterator&) noexcept = default;
	constexpr multimap_value_iterator(multimap_value_iterator&&) noexcept = default;
	constexpr multimap_value_iterator& operator=(const multimap_value_iterator&) noexcept = default;
	constexpr multimap_value_iterator& operator=(multimap_value_iterator&&) noexcept = default;
	~multimap_value_iterator() noexcept = default;

	constexpr multimap_value_iterator& operator++() noexcept {
		++_iterator;
		return *this;
	}

	constexpr multimap_value_iterator& operator--() noexcept {
		--_iterator;
		return *this;
	}

	constexpr multimap_value_iterator operator++(int) noexcept {
		const auto old = *this;
		++*this;
		return old;
	}

	constexpr multimap_value_iterator operator--(int) noexcept {
		const auto old = *this;
		--*this;
		return old;
	}

	constexpr ValueType& operator*() const noexcept {
		return _iterator->second;
	}

	constexpr ValueType* operator->() const noexcept {
		return std::addressof(_iterator->second);
	}

private:
	ActualMultimapIterator _iterator;
};

template <typename ActualMultimapIterator>
multimap_value_iterator(const ActualMultimapIterator&) -> multimap_value_iterator<decltype (std::iterator_traits<ActualMultimapIterator>::value_type::second), ActualMultimapIterator>;
