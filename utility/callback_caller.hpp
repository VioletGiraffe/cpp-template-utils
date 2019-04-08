#pragma once

#include "../container/algorithms.hpp"

#include <algorithm>
#include <assert.h>
#include <vector>

template <class Interface>
class CallbackCaller {
public:
	void addSubscriber(Interface* instance)
	{
		assert(std::find(_subscribers.begin(), _subscribers.end(), instance) == _subscribers.end());
		_subscribers.push_back(instance);
	}

	void removeSubscriber(Interface* instance)
	{
		ContainerAlgorithms::erase_all_occurrences(_subscribers, instance);
	}

	template <typename MethodPointer, typename ...Args>
	void invokeCallback(MethodPointer methodPtr, Args... args) const
	{
		for (Interface* subscriber: _subscribers)
			(subscriber->*methodPtr)(std::forward<Args>(args)...);
	}

protected:
	std::vector<Interface*> _subscribers;
};
