#pragma once

#include <algorithm>
#include <assert.h>
#include <functional>

template <class ContainerType, typename Comparator = std::less<> /* transparent (heterogenous) comparator */>
class ordered_container : public ContainerType
{
public:
	template <typename T>
	[[nodiscard]] typename ContainerType::iterator find(const T& value);

	template <typename T>
	[[nodiscard]] typename ContainerType::const_iterator find(const T& value) const;

	void sort();

private:
	// Shared by both find() overloads: SelfType carries the constness into the iterator type
	template <typename SelfType, typename T>
	static auto find_impl(SelfType& self, const T& value)
	{
		const auto end_iterator = self.end();
		// Verified rather than tracked: mutations through the public base cannot be intercepted
		assert(std::is_sorted(self.begin(), end_iterator, Comparator()));

		const auto it = std::lower_bound(self.begin(), end_iterator, value, Comparator());
		return it != end_iterator && Comparator()(value, *it) == false ? it : end_iterator;
	}
};

template <class ContainerType, typename Comparator> template <typename T>
typename ContainerType::iterator ordered_container<ContainerType, Comparator>::find(const T& value)
{
	return find_impl(*this, value);
}

template <class ContainerType, typename Comparator> template <typename T>
typename ContainerType::const_iterator ordered_container<ContainerType, Comparator>::find(const T& value) const
{
	return find_impl(*this, value);
}


template <class ContainerType, typename Comparator /*= std::less<> */>
void ordered_container<ContainerType, Comparator>::sort()
{
	std::sort(ContainerType::begin(), ContainerType::end(), Comparator());
}
