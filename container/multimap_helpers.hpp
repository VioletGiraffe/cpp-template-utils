#pragma once

#include <memory>
#include <type_traits>

template <typename ActualMultimapIterator>
struct multimap_value_iterator {
	using value_type = std::remove_reference_t<decltype(ActualMultimapIterator::value_type::second)>;
	using iterator_category = typename ActualMultimapIterator::iterator_category;
	using difference_type = typename ActualMultimapIterator::difference_type;
	using pointer = value_type*;
	using reference = value_type&;

	constexpr multimap_value_iterator(const ActualMultimapIterator& iterator) noexcept
		: _iterator{iterator}
	{}

	constexpr multimap_value_iterator() noexcept = default;
	constexpr multimap_value_iterator(const multimap_value_iterator&) noexcept = default;
	constexpr multimap_value_iterator(multimap_value_iterator&&) noexcept = default;
	constexpr multimap_value_iterator& operator=(const multimap_value_iterator&) noexcept = default;
	constexpr multimap_value_iterator& operator=(multimap_value_iterator&&) noexcept = default;

	constexpr auto nativeIterator() const noexcept {
		return _iterator;
	}

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

	// TODO: why does it fail with value_type instead of auto?
	constexpr auto& operator*() const noexcept {
		return _iterator->second;
	}

	constexpr auto* operator->() const noexcept {
		return std::addressof(_iterator->second);
	}

	constexpr bool operator==(const multimap_value_iterator& other) const noexcept
	{
		return _iterator == other._iterator;
	}

private:
	ActualMultimapIterator _iterator;
};
