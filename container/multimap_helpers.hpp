#pragma once

#include <iterator>
#include <memory>

template <typename ActualMultimapIterator>
struct multimap_value_iterator {
	using value_type = decltype(std::iterator_traits<ActualMultimapIterator>::value_type::second);

	constexpr multimap_value_iterator(const ActualMultimapIterator& iterator) noexcept
		: _iterator{iterator}
	{}

	constexpr multimap_value_iterator() = default;
	constexpr multimap_value_iterator(const multimap_value_iterator&) noexcept = default;
	constexpr multimap_value_iterator(multimap_value_iterator&&) noexcept = default;
	constexpr multimap_value_iterator& operator=(const multimap_value_iterator&) noexcept = default;
	constexpr multimap_value_iterator& operator=(multimap_value_iterator&&) noexcept = default;

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

	constexpr value_type& operator*() const noexcept {
		return _iterator->second;
	}

	constexpr value_type* operator->() const noexcept {
		return std::addressof(_iterator->second);
	}

	constexpr bool operator==(const multimap_value_iterator& other) const noexcept {
		return _iterator == other._iterator;
	}

private:
	ActualMultimapIterator _iterator;
};
