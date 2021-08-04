#pragma once
#include <utility>

template <typename T, int>
struct NamedType
{
	explicit constexpr NamedType(T v) noexcept : _value{ std::move(v) } {}
	constexpr NamedType() noexcept = default;

	constexpr operator T() const noexcept
	{
		return _value;
	}

private:
	T _value{}; // default initialization for class types; zero initialization for primitive type: 'false' for bool, '0' for int etc.
};

#define UniqueNamedBoolType NamedType<bool, __LINE__>
#define UniqueNamedType(T) NamedType<T, __LINE__>
