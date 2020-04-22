#pragma once

#include <assert.h>
#include <stdlib.h>

template <class T>
struct MemoryTrackingAllocator
{
	using value_type = T;
	using propagate_on_container_move_assignment = std::true_type;

	MemoryTrackingAllocator() = default;
	MemoryTrackingAllocator(const MemoryTrackingAllocator&) noexcept = delete;
	template <typename U>
	MemoryTrackingAllocator(const MemoryTrackingAllocator<U>&) noexcept = delete;
	MemoryTrackingAllocator(MemoryTrackingAllocator&& other) noexcept = default;

	~MemoryTrackingAllocator() {
		//assert(_allocatedItemsCounter == 0);
	}

	MemoryTrackingAllocator& operator=(const MemoryTrackingAllocator&) = delete;

	MemoryTrackingAllocator& operator=(MemoryTrackingAllocator&& other) noexcept = default;

	[[nodiscard]] T* allocate(const size_t n) noexcept
	{
		auto p = static_cast<T*>(::malloc(n * sizeof(T)));
		if (p)
			_allocatedItemsCounter += n;
		else
			assert(false);

		return p;
	}

	void deallocate(T* p, size_t n) noexcept
	{
		if (p)
		{
			assert(_allocatedItemsCounter >= n);
			_allocatedItemsCounter -= n;
			::free(p);
		}
	}

	size_t allocatedItemsCount() const
	{
		return _allocatedItemsCounter;
	}

	size_t allocatedBytesCount() const
	{
		return allocatedItemsCount() * sizeof(T);
	}

private:
	size_t _allocatedItemsCounter = 0;
};

template <class T, class U>
constexpr bool operator==(const MemoryTrackingAllocator<T>&, const MemoryTrackingAllocator<U>&) { return true; }
template <class T, class U>
constexpr bool operator!=(const MemoryTrackingAllocator<T>&, const MemoryTrackingAllocator<U>&) { return false; }
