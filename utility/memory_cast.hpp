#pragma once

#include <memory>
#include <string.h>
#include <type_traits>
#include <utility>

template <typename TargetType, typename SourceType, typename... TargetType_Constructor_Arguments>
constexpr TargetType memory_cast(SourceType source, TargetType_Constructor_Arguments&& ...constructorArguments) noexcept
{
	static_assert(!std::is_reference_v<TargetType>);
	static_assert(!std::is_reference_v<SourceType>);
	static_assert(std::is_same_v<TargetType, std::decay_t<TargetType>>);
	static_assert(std::is_trivially_copyable_v<TargetType>);

	TargetType value{std::forward<TargetType_Constructor_Arguments>(constructorArguments)...};
	if constexpr (std::is_pointer_v<SourceType>)
		::memcpy(std::addressof(value), source, sizeof(value));
	else
	{
		static_assert(sizeof(SourceType) == sizeof(TargetType));
		::memcpy(std::addressof(value), std::addressof(source), sizeof(value));
	}

	return value;
}
