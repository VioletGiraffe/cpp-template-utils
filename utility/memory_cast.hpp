#pragma once

#include "../utility/extra_type_traits.hpp"

#include <memory>
#include <string.h>
#include <utility>

template <typename TargetType, typename SourceType, typename... TargetType_Constructor_Arguments>
constexpr TargetType memory_cast(SourceType source, TargetType_Constructor_Arguments&& ...constructorArguments) noexcept
{
	static_assert((!std::is_pointer_v<std::remove_cv_t<SourceType>> && is_trivially_serializable_v<SourceType>) || (std::is_pointer_v< std::remove_cv_t<SourceType>> && is_trivially_serializable_v<std::remove_pointer_t<std::remove_cv_t<SourceType>>>));
	static_assert(is_trivially_serializable_v<TargetType>);

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

template <typename T1, typename T2>
constexpr bool bitwise_equal(T1 v1, T2 v2)
{
	static_assert(sizeof(T1) == sizeof(T2));
	static_assert(sizeof(T1) <= 8, "This is odd");
	static_assert(is_trivially_serializable_v<T1> && is_trivially_serializable_v<T2>);

	bool equal = true;
	static_for<0, sizeof(T1)>([&](auto byteNumber) {
		const int byteIndex = byteNumber;
		if (((v1 >> byteIndex) & 0xFF) != ((v2 >> byteIndex) & 0xFF))
			equal = false;
	});

	return equal;
}
