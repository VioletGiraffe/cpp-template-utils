#pragma once

#include "../container/algorithms.hpp"

#include <assert.h>
#include <algorithm>
#include <vector>

#define INVOKE_CALLBACK(callback, ...) {for (auto& s: _subscribers) {s->callback(__VA_ARGS__);}}

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

protected:
	std::vector<Interface*> _subscribers;
};
