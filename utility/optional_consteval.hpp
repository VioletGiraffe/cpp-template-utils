#pragma once

template <typename T>
class optional_consteval {
public:
	consteval optional_consteval() noexcept = default;
	consteval optional_consteval(const T& value) noexcept :
		_value{value},
		_valid{true}
	{}

	consteval optional_consteval& operator=(const optional_consteval& other) noexcept = default;
	consteval optional_consteval& operator=(const T& value) noexcept
	{
		_value = value;
		_valid = true;
		return *this;
	}

	[[nodiscard]] consteval bool operator==(const optional_consteval& other) const noexcept
	{
		if (!_valid || !other._valid)
			return false;

		return _value == other._value;
	}

	[[nodiscard]] consteval bool operator==(const T& value) const noexcept
	{
		return _valid && _value == value;
	}

	[[nodiscard]] consteval bool has_value() const noexcept
	{
		return _valid;
	}

	[[nodiscard]] explicit consteval operator bool() const noexcept
	{
		return has_value();
	}

	[[nodiscard]] consteval T value() const
	{
		if (!_valid)
			throw "This optional holds no value!";

		return _value;
	}

	[[nodiscard]] consteval T operator*() const
	{
		return value();
	}

private:
	T _value {};
	bool _valid = false;
};
