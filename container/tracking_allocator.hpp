#pragma once
#include <memory>

template<class T>
struct TrackingAllocator {
	using value_type = T;
	using pointer = T*;

	using is_always_equal = std::true_type;
	using propagate_on_container_move_assignment = std::false_type;
	using propagate_on_container_copy_assignment = std::false_type;

	template<class U> friend struct TrackingAllocator;

	constexpr TrackingAllocator() noexcept = default;
	constexpr TrackingAllocator(const TrackingAllocator& other) noexcept = default;

	template<class U>
	explicit constexpr TrackingAllocator(U&& t) noexcept :
		bytes_{t.bytes_}
	{
	}

	constexpr TrackingAllocator& operator=(const TrackingAllocator& other) noexcept = delete;

	constexpr TrackingAllocator select_on_container_copy_construction() const noexcept {
		return TrackingAllocator{};
	}

	constexpr bool operator=(const TrackingAllocator&) const noexcept
	{
		return true;
	}

	constexpr bool operator!=(const TrackingAllocator& other) const noexcept
	{
		return !operator=(other);
	}

	[[nodiscard]] pointer allocate(std::size_t n) {
		bytes_ += sizeof(T) * n;
		return std::allocator<T>{}.allocate(n);
	}

	void deallocate(pointer p, std::size_t n) {
		bytes_ -= sizeof(T) * n;
		std::allocator<T>{}.deallocate(p, n);
	}

	[[nodiscard]] constexpr std::size_t bytes() const noexcept {
		return bytes_;
	}

private:
	std::size_t bytes_ = 0;
};
