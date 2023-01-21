#pragma once

#include <stdint.h>
#include <stddef.h> // size_t

[[nodiscard]] inline constexpr uint64_t operator "" _u64(unsigned long long value)
{
	return static_cast<uint64_t>(value);
}

[[nodiscard]] inline constexpr size_t operator "" _z(unsigned long long value)
{
	return static_cast<size_t>(value);
}

[[nodiscard]] inline constexpr int64_t operator "" _i64(unsigned long long value)
{
	return static_cast<int64_t>(value);
}

[[nodiscard]] inline constexpr uint16_t operator "" _u16(unsigned long long value)
{
	return static_cast<uint16_t>(value);
}

[[nodiscard]] inline constexpr int16_t operator "" _i16(unsigned long long value)
{
	return static_cast<int16_t>(value);
}
