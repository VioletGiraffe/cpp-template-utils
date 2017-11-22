#pragma once

#include <utility>

template <class ContainerType>
class ordered_container : public ContainerType
{
public:
	template <typename T>
	typename ContainerType::iterator find(const T& value);

	template <typename T>
	std::pair<typename ContainerType::iterator, bool> insert(const T& value);
};

template<typename T>
ContainerType::iterator ordered_container::find(const ordered_container::T& value)
{

}

template<typename T>
std::pair<ContainerType::iterator, bool> ordered_container::insert(const ordered_container::T& value)
{

}
