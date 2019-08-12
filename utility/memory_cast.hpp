#pragma once

#include <type_traits>
#include <string.h>

namespace util {

template <typename TargetType>
TargetType memory_cast(const void* const memoryPtr) noexcept
{
	static_assert(!std::is_reference_v<TargetType>);
	static_assert(std::is_standard_layout_v<TargetType>);
	static_assert(std::is_trivial_v<TargetType>);

	TargetType value;
	::memcpy(&value, memoryPtr, sizeof(value));

	return value;
}

}
