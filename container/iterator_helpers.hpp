#pragma once

#include <assert.h>

template<typename Container>
struct const_forward_iterator_wrapper
{
	const_forward_iterator_wrapper() {}
	const_forward_iterator_wrapper(const Container& container, const typename Container::const_iterator& iterator): _iterator(iterator), _container(&container) {}

	const_forward_iterator_wrapper& operator=(const typename Container::const_iterator it)
	{
		assert(valid());
		_iterator = it;
		return *this;
	}

	const_forward_iterator_wrapper& operator++()
	{
		assert(valid());
		++_iterator;

		return *this;
	}

	const typename Container::value_type& operator*() const
	{
		assert(valid());
		return *_iterator;
	}

	typename Container::value_type const * operator->() const
	{
		assert(valid());
		return &(*_iterator);
	}

	bool endReached() const
	{
		assert(valid());
		return _iterator >= _container->cend();
	}

	bool valid() const
	{
		return _container != nullptr;
	}

	bool operator==(const typename Container::const_iterator other) const
	{
		assert(valid());
		return _iterator == other;
	}

	bool operator==(const const_forward_iterator_wrapper other) const
	{
		assert(valid() && _container == other._container);
		return _iterator == other._iterator;
	}

	bool operator!=(const typename Container::const_iterator other) const
	{
		return !(*this == other);
	}

	bool operator!=(const const_forward_iterator_wrapper other) const
	{
		return !(*this == other);
	}

	typename Container::const_iterator _iterator;
	const Container* _container = nullptr;
};

namespace forward_iterator_wrapper {

	template<typename Container>
	const_forward_iterator_wrapper<Container> cbegin(const Container& c)
	{
		const_forward_iterator_wrapper<Container> it;
		it._container = &c;
		it._iterator = c.cbegin();
		return it;
	}

	template<typename Container>
	const_forward_iterator_wrapper<Container> cend(const Container& c)
	{
		const_forward_iterator_wrapper<Container> it;
		it._container = &c;
		it._iterator = c.cend();
		return it;
	}
}

