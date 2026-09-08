#pragma once

#include "extra_type_traits.hpp"

#include <bit>
#include <memory> // std::addressof
#include <string.h>

// is_trivially_serializable_v rejects pointers and arrays: those select the overload below
template <typename TargetType, typename SourceType> requires is_trivially_serializable_v<SourceType>
[[nodiscard]] constexpr TargetType memory_cast(const SourceType& source) noexcept
{
	static_assert(is_trivially_serializable_v<TargetType>, "TargetType must be trivially serializable");

	return std::bit_cast<TargetType>(source);
}

// Requires at least sizeof(TargetType) readable bytes at source
// Not constexpr: memcpy cannot be constant-evaluated
template <typename TargetType, typename SourceType>
[[nodiscard]] TargetType memory_cast(const SourceType* const source) noexcept
{
	static_assert(is_trivially_serializable_v<TargetType>, "TargetType must be trivially serializable");
	static_assert(std::is_void_v<SourceType> || is_trivially_serializable_v<SourceType>, "The source must point to raw memory or to a trivially serializable object");

	TargetType value;
	::memcpy(std::addressof(value), source, sizeof(value));
	return value;
}

template <typename T>
inline void zero_object(T& object) noexcept
{
	::memset(std::addressof(object), 0, sizeof(object));
}
