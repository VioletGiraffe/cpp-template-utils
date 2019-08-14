#pragma once

#include <string.h>
#include <type_traits>

namespace util {

template <typename TargetType>
TargetType memory_cast(const void* const memoryPtr) noexcept
{
	static_assert(!std::is_reference_v<TargetType>);
	static_assert(std::is_trivial_v<TargetType>);

	TargetType value;
	::memcpy(&value, memoryPtr, sizeof(value));

	return value;
}

}
