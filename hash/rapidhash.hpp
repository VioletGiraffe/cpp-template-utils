#pragma once

#include "../compiler/compiler_warnings_control.h"
#include "../utility/extra_type_traits.hpp"

DISABLE_COMPILER_WARNINGS
#include "rapidhash.h"
RESTORE_COMPILER_WARNINGS

#include <memory> // std::addressof

#include <stddef.h>
#include <stdint.h>

[[nodiscard]] inline uint64_t rapidhash64(const void* data, size_t len) noexcept
{
	return rapidhash_detail::rapidhash(data, len);
}

[[nodiscard]] inline uint64_t rapidhash64(const void* data, size_t len, uint64_t seed) noexcept
{
	return rapidhash_detail::rapidhash_withSeed(data, len, seed);
}

// rapidhash64v hashes the whole object representation, padding bytes included: two objects with equal members but
// different padding hash differently, and no trait can rule that out.

template <typename T>
[[nodiscard]] inline uint64_t rapidhash64v(T&& value) noexcept
{
	static_assert(is_trivially_serializable_v<remove_cv_and_reference_t<T>>, "Hashing an object by its representation requires a trivially serializable type");
	return rapidhash64(std::addressof(value), sizeof(value));
}

template <typename T>
[[nodiscard]] inline uint64_t rapidhash64v(T&& value, uint64_t seed) noexcept
{
	static_assert(is_trivially_serializable_v<remove_cv_and_reference_t<T>>, "Hashing an object by its representation requires a trivially serializable type");
	return rapidhash64(std::addressof(value), sizeof(value), seed);
}
