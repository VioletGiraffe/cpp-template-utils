#pragma once

#include <algorithm>
#include <assert.h>
#include <utility>
#include <vector>

template <class Interface>
class CallbackCaller {
public:
	void addSubscriber(Interface* instance)
	{
		assert(!isSubscribed(instance));
		_subscribers.push_back(instance);
	}

	void removeSubscriber(Interface* instance)
	{
		std::erase(_subscribers, instance);
	}

	template <typename MethodPointer, typename ...Args>
	void invokeCallback(MethodPointer methodPtr, Args... args) const
	{
		// A callback may subscribe or unsubscribe, so the loop walks a snapshot and re-checks membership immediately
		// before every call: an instance removed by an earlier callback must not be called, and one added during the
		// notification must not receive the event in flight.
		const std::vector<Interface*> snapshot = _subscribers;

		// args are by-value copies, not forwarding references, so std::forward moves out of them: only the last
		// surviving subscriber may receive that. Each call therefore lags one snapshot entry behind the loop.
		Interface* pending = nullptr;
		for (Interface* const subscriber: snapshot)
		{
			if (pending && isSubscribed(pending))
				(pending->*methodPtr)(args...);

			pending = subscriber;
		}

		if (pending && isSubscribed(pending))
			(pending->*methodPtr)(std::forward<Args>(args)...);
	}

protected:
	[[nodiscard]] bool isSubscribed(Interface* instance) const
	{
		return std::find(_subscribers.cbegin(), _subscribers.cend(), instance) != _subscribers.cend();
	}

	std::vector<Interface*> _subscribers;
};
