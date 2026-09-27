#pragma once

#include "mixers.h"

#include <stdint.h>
#include <type_traits>

// Hash functors for integer keys in hash tables.
// is_avalanching: ankerl::unordered_dense and boost::unordered use the result as is, without mixing it again.

// For keys that are already hashes
struct identity_hash {
	using is_avalanching = std::true_type;

	[[nodiscard]] inline constexpr uint64_t operator()(uint64_t key) const noexcept {
		return key;
	}
};

// For keys whose bits are not well spread, such as packed fields or counters; mixes the same way in every container
struct moremur_hash {
	using is_avalanching = std::true_type;

	[[nodiscard]] inline uint64_t operator()(uint64_t key) const noexcept {
		return mix_moremur(key);
	}
};
