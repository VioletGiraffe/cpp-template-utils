#pragma once
#include "../hash/hash_consteval.hpp"

#include <type_traits>

template <typename T, uint64_t>
struct NamedType
{
	static_assert(std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>); // Not a hard requirement, but right now the wrapper is not suitable for non-trivial types

	explicit constexpr NamedType(T v) noexcept : _value{ v } {}
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
