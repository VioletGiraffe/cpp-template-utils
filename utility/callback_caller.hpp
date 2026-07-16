#pragma once

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
		std::erase(_subscribers, instance);
	}

	template <typename MethodPointer, typename ...Args>
	void invokeCallback(MethodPointer methodPtr, Args... args) const
	{
		// args are already by-value copies, not forwarding references, so std::forward would move out of them. Only the
		// last subscriber can safely receive the moved value - every earlier one still needs args intact for the next call.
		const size_t nSubscribers = _subscribers.size();
		if (nSubscribers == 0)
			return;

		for (size_t i = 0; i < nSubscribers - 1; ++i)
			(_subscribers[i]->*methodPtr)(args...);

		(_subscribers[nSubscribers - 1]->*methodPtr)(std::forward<Args>(args)...);
	}

protected:
	std::vector<Interface*> _subscribers;
};
