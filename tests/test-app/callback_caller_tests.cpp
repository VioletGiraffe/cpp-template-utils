#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include "3rdparty/catch2/catch.hpp"
RESTORE_COMPILER_WARNINGS

#include "utility/callback_caller.hpp"

#include <functional>
#include <vector>

namespace {

	struct Payload
	{
		int copies = 0;
		int moves = 0;
		bool movedFrom = false;

		Payload() = default;
		Payload(const Payload& other) noexcept : copies(other.copies + 1), moves(other.moves) {}
		Payload(Payload&& other) noexcept : copies(other.copies), moves(other.moves + 1) { other.movedFrom = true; }
	};

	struct Listener
	{
		virtual ~Listener() = default;

		virtual void onEvent(int eventId) = 0;
		virtual void onPayload(Payload payload) = 0;
	};

	struct Recorder : Listener
	{
		std::vector<int> receivedEvents;
		int payloadCopies = 0;
		int payloadMoves = 0;

		// Runs inside every callback: the tests use it to subscribe or unsubscribe mid-notification
		std::function<void()> duringCallback;

		void subscribe(CallbackCaller<Listener>& caller, std::vector<int>& log, const int recorderId)
		{
			_id = recorderId;
			_callLog = &log;
			caller.addSubscriber(this);
		}

		void onEvent(const int eventId) override
		{
			receivedEvents.push_back(eventId);
			recordCall();
		}

		void onPayload(Payload payload) override
		{
			payloadCopies = payload.copies;
			payloadMoves = payload.moves;
			recordCall();
		}

	private:
		void recordCall()
		{
			_callLog->push_back(_id);
			if (duringCallback)
				duringCallback();
		}

		std::vector<int>* _callLog = nullptr;
		int _id = 0;
	};
}

TEST_CASE("CallbackCaller - every subscriber is called, in subscription order", "[callback_caller]")
{
	CallbackCaller<Listener> caller;
	std::vector<int> callLog;

	// An empty subscriber list has no last element to hand the arguments to
	caller.invokeCallback(&Listener::onEvent, 1);
	CHECK(callLog.empty());

	Recorder a, b, c;
	a.subscribe(caller, callLog, 1);
	b.subscribe(caller, callLog, 2);
	c.subscribe(caller, callLog, 3);

	caller.invokeCallback(&Listener::onEvent, 7);

	CHECK(callLog == std::vector<int>{ 1, 2, 3 });
	CHECK(a.receivedEvents == std::vector<int>{ 7 });
	CHECK(b.receivedEvents == std::vector<int>{ 7 });
	CHECK(c.receivedEvents == std::vector<int>{ 7 });
}

TEST_CASE("CallbackCaller - removeSubscriber", "[callback_caller]")
{
	CallbackCaller<Listener> caller;
	std::vector<int> callLog;

	Recorder a, b;
	a.subscribe(caller, callLog, 1);
	b.subscribe(caller, callLog, 2);

	caller.removeSubscriber(&a);
	caller.invokeCallback(&Listener::onEvent, 1);
	CHECK(callLog == std::vector<int>{ 2 });

	// Removing an instance that never subscribed leaves the list alone
	Recorder stranger;
	caller.removeSubscriber(&stranger);
	callLog.clear();
	caller.invokeCallback(&Listener::onEvent, 2);
	CHECK(callLog == std::vector<int>{ 2 });

	caller.removeSubscriber(&b);
	callLog.clear();
	caller.invokeCallback(&Listener::onEvent, 3);
	CHECK(callLog.empty());
}

TEST_CASE("CallbackCaller - the subscriber list is mutated from inside a callback", "[callback_caller]")
{
	CallbackCaller<Listener> caller;
	std::vector<int> callLog;
	Recorder a, b, c;

	SECTION("a callback removes a subscriber that has not been called yet")
	{
		a.subscribe(caller, callLog, 1);
		b.subscribe(caller, callLog, 2);
		c.subscribe(caller, callLog, 3);
		a.duringCallback = [&] { caller.removeSubscriber(&b); };

		caller.invokeCallback(&Listener::onEvent, 1);

		CHECK(callLog == std::vector<int>{ 1, 3 });
		CHECK(b.receivedEvents.empty());
	}

	SECTION("a callback removes the last subscriber")
	{
		a.subscribe(caller, callLog, 1);
		b.subscribe(caller, callLog, 2);
		a.duringCallback = [&] { caller.removeSubscriber(&b); };

		caller.invokeCallback(&Listener::onEvent, 1);

		CHECK(callLog == std::vector<int>{ 1 });
	}

	SECTION("a callback removes every subscriber after it")
	{
		a.subscribe(caller, callLog, 1);
		b.subscribe(caller, callLog, 2);
		c.subscribe(caller, callLog, 3);
		a.duringCallback = [&] { caller.removeSubscriber(&b); caller.removeSubscriber(&c); };

		caller.invokeCallback(&Listener::onEvent, 1);

		CHECK(callLog == std::vector<int>{ 1 });
	}

	SECTION("a callback removes itself")
	{
		a.subscribe(caller, callLog, 1);
		b.subscribe(caller, callLog, 2);
		a.duringCallback = [&] { caller.removeSubscriber(&a); };

		caller.invokeCallback(&Listener::onEvent, 1);
		CHECK(callLog == std::vector<int>{ 1, 2 });

		// The removal took effect: the next notification skips it
		callLog.clear();
		caller.invokeCallback(&Listener::onEvent, 2);
		CHECK(callLog == std::vector<int>{ 2 });
		CHECK(a.receivedEvents == std::vector<int>{ 1 });
	}

	SECTION("a callback adds a subscriber")
	{
		a.subscribe(caller, callLog, 1);
		b.subscribe(caller, callLog, 2);

		Recorder late;
		bool alreadyAdded = false;
		a.duringCallback = [&] {
			if (!alreadyAdded)
			{
				alreadyAdded = true;
				late.subscribe(caller, callLog, 9);
			}
		};

		// The notification in flight was snapshotted before the new subscriber existed
		caller.invokeCallback(&Listener::onEvent, 1);
		CHECK(callLog == std::vector<int>{ 1, 2 });
		CHECK(late.receivedEvents.empty());

		callLog.clear();
		caller.invokeCallback(&Listener::onEvent, 2);
		CHECK(callLog == std::vector<int>{ 1, 2, 9 });
	}
}

// A value reaches a callback through two constructions: one into invokeCallback's by-value parameter, one into the callback's own.
// An lvalue argument arrives as copies 2, moves 0; the last subscriber gets copies 1, moves 1 instead.
// An rvalue argument is elided into the parameter: the subscriber it is moved into sees copies 0, moves 1.
TEST_CASE("CallbackCaller - only the last subscriber receives moved arguments", "[callback_caller]")
{
	CallbackCaller<Listener> caller;
	std::vector<int> callLog;
	Recorder a, b, c;

	SECTION("a lone subscriber is the last one")
	{
		a.subscribe(caller, callLog, 1);

		Payload payload;
		caller.invokeCallback(&Listener::onPayload, payload);

		CHECK(a.payloadCopies == 1);
		CHECK(a.payloadMoves == 1);

		// invokeCallback moves out of its own parameter, never out of the caller's argument
		CHECK(payload.movedFrom == false);
		CHECK(payload.copies == 0);
	}

	SECTION("every earlier subscriber receives a copy")
	{
		a.subscribe(caller, callLog, 1);
		b.subscribe(caller, callLog, 2);
		c.subscribe(caller, callLog, 3);

		Payload payload;
		caller.invokeCallback(&Listener::onPayload, payload);

		CHECK(a.payloadCopies == 2);
		CHECK(a.payloadMoves == 0);
		CHECK(b.payloadCopies == 2);
		CHECK(b.payloadMoves == 0);
		CHECK(c.payloadCopies == 1);
		CHECK(c.payloadMoves == 1);
		CHECK(payload.movedFrom == false);
	}

	SECTION("an rvalue argument is elided into the parameter and moved from there")
	{
		a.subscribe(caller, callLog, 1);

		caller.invokeCallback(&Listener::onPayload, Payload{});

		CHECK(a.payloadCopies == 0);
		CHECK(a.payloadMoves == 1);
	}

	SECTION("nothing is moved into when the last subscriber leaves mid-notification")
	{
		a.subscribe(caller, callLog, 1);
		b.subscribe(caller, callLog, 2);
		a.duringCallback = [&] { caller.removeSubscriber(&b); };

		Payload payload;
		caller.invokeCallback(&Listener::onPayload, payload);

		CHECK(callLog == std::vector<int>{ 1 });

		// a was called from the loop body, before b's departure was known
		CHECK(a.payloadCopies == 2);
		CHECK(a.payloadMoves == 0);
	}
}

TEST_CASE("CallbackCaller - a callback starts another notification", "[callback_caller]")
{
	CallbackCaller<Listener> caller;
	std::vector<int> callLog;

	Recorder a, b;
	a.subscribe(caller, callLog, 1);
	b.subscribe(caller, callLog, 2);

	bool reentered = false;
	a.duringCallback = [&] {
		if (reentered)
			return;

		reentered = true;
		caller.invokeCallback(&Listener::onEvent, 2);
	};

	caller.invokeCallback(&Listener::onEvent, 1);

	// The nested notification runs to completion before the outer one resumes
	CHECK(callLog == std::vector<int>{ 1, 1, 2, 2 });
	CHECK(a.receivedEvents == std::vector<int>{ 1, 2 });
	CHECK(b.receivedEvents == std::vector<int>{ 2, 1 });
}
