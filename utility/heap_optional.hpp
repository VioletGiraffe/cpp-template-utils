#pragma once

#include <memory>
#include <optional>
#include <utility>

// An optional value kept on the heap: the object is one pointer wide whether or not it holds a value.
// Copies deep-copy the value and == compares values, as with std::optional.
// Unlike std::optional, a moved-from heap_optional is empty.
template <typename T>
class heap_optional {
public:
	heap_optional() noexcept = default;
	heap_optional(std::nullopt_t) noexcept {}
	heap_optional(const T& value) : _value{ std::make_unique<T>(value) } {}
	heap_optional(T&& value) : _value{ std::make_unique<T>(std::move(value)) } {}

	heap_optional(const heap_optional& other) : _value{ other._value ? std::make_unique<T>(*other._value) : nullptr } {}
	heap_optional(heap_optional&&) noexcept = default;

	heap_optional& operator=(const heap_optional& other)
	{
		if (!other._value)
			_value.reset();
		else
			*this = *other._value;

		return *this;
	}

	heap_optional& operator=(heap_optional&&) noexcept = default;

	heap_optional& operator=(std::nullopt_t) noexcept
	{
		_value.reset();
		return *this;
	}

	// Reuses the allocation when a value is already held
	heap_optional& operator=(const T& value)
	{
		if (_value)
			*_value = value;
		else
			_value = std::make_unique<T>(value);

		return *this;
	}

	heap_optional& operator=(T&& value)
	{
		if (_value)
			*_value = std::move(value);
		else
			_value = std::make_unique<T>(std::move(value));

		return *this;
	}

	void reset() noexcept { _value.reset(); }

	[[nodiscard]] bool has_value() const noexcept { return _value != nullptr; }
	[[nodiscard]] explicit operator bool() const noexcept { return has_value(); }

	[[nodiscard]] T& operator*() noexcept { return *_value; }
	[[nodiscard]] const T& operator*() const noexcept { return *_value; }
	[[nodiscard]] T* operator->() noexcept { return _value.get(); }
	[[nodiscard]] const T* operator->() const noexcept { return _value.get(); }

	[[nodiscard]] bool operator==(const heap_optional& other) const
	{
		if (has_value() != other.has_value())
			return false;

		return !has_value() || *_value == *other._value;
	}

private:
	std::unique_ptr<T> _value;
};
