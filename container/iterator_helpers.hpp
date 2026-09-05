#pragma once

#include <assert.h>
#include <iterator>
#include <type_traits>

template<typename Container>
struct const_forward_iterator_wrapper
{
	using underlying_iterator = typename Container::const_iterator;

	using iterator_category = std::forward_iterator_tag;
	using value_type = typename std::iterator_traits<underlying_iterator>::value_type;
	using difference_type = typename std::iterator_traits<underlying_iterator>::difference_type;
	using pointer = typename std::iterator_traits<underlying_iterator>::pointer;
	using reference = typename std::iterator_traits<underlying_iterator>::reference;

	constexpr const_forward_iterator_wrapper() noexcept = default;
	constexpr const_forward_iterator_wrapper(const Container& container, const underlying_iterator& iterator): _iterator(iterator), _container(&container) {}

	// Retains a pointer to the container, so a temporary would leave it dangling
	const_forward_iterator_wrapper(const Container&&, const underlying_iterator&) = delete;

	const_forward_iterator_wrapper& operator=(const underlying_iterator it)
	{
		assert(isBound());
		_iterator = it;
		return *this;
	}

	const_forward_iterator_wrapper& operator++()
	{
		assert(isBound());
		++_iterator;

		return *this;
	}

	const_forward_iterator_wrapper operator++(int)
	{
		const auto previous = *this;
		++(*this);

		return previous;
	}

	reference operator*() const
	{
		assert(isBound());
		return *_iterator;
	}

	// Delegates to the underlying iterator: &(*_iterator) is the address of a temporary for a container with proxy references
	pointer operator->() const requires (std::is_pointer_v<underlying_iterator> || requires (const underlying_iterator& it) { it.operator->(); })
	{
		assert(isBound());
		if constexpr (std::is_pointer_v<underlying_iterator>)
			return _iterator;
		else
			return _iterator.operator->();
	}

	[[nodiscard]] bool endReached() const
	{
		assert(isBound());
		return _iterator == _container->cend();
	}

	// Not a check for iterator invalidation: only reports whether a container was supplied
	[[nodiscard]] bool isBound() const
	{
		return _container != nullptr;
	}

	[[nodiscard]] bool operator==(const underlying_iterator other) const
	{
		assert(isBound());
		return _iterator == other;
	}

	[[nodiscard]] bool operator==(const const_forward_iterator_wrapper& other) const
	{
		assert(isBound() && _container == other._container);
		return _iterator == other._iterator;
	}

	underlying_iterator _iterator{};
	const Container* _container = nullptr;
};

namespace forward_iterator_wrapper {

	template<typename Container>
	[[nodiscard]] const_forward_iterator_wrapper<Container> cbegin(const Container& c)
	{
		return { c, c.cbegin() };
	}

	template<typename Container>
	[[nodiscard]] const_forward_iterator_wrapper<Container> cend(const Container& c)
	{
		return { c, c.cend() };
	}

	// A temporary container would leave the returned wrapper dangling
	template<typename Container>
	void cbegin(const Container&&) = delete;

	template<typename Container>
	void cend(const Container&&) = delete;
} // namespace forward_iterator_wrapper
