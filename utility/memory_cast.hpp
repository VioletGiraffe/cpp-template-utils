#pragma once

#include <memory>
#include <string.h>
#include <type_traits>
#include <utility>

namespace util {

template <typename TargetType, typename... TargetType_Constructor_Arguments>
TargetType memory_cast(const void* const memoryPtr, TargetType_Constructor_Arguments&& ...constructorArguments) noexcept
{
	static_assert(!std::is_reference_v<TargetType>);
	static_assert(std::is_trivially_copyable_v<TargetType>);

	TargetType value{std::forward<TargetType_Constructor_Arguments>(constructorArguments)...};
	::memcpy(std::addressof(value), memoryPtr, sizeof(value));

	return value;
}

}
