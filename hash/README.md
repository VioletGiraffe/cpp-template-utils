# Hash utilities

Compile-time and runtime non-cryptographic hashing helpers. These functions are suitable for hash tables, identifiers, partitioning, and checks against accidental changes; they are not authentication or password-hashing primitives.

| Header | Facility |
|---|---|
| `hash_consteval.hpp` | `murmur3_32_consteval()` computes seedable MurmurHash3 x86-32 for a `std::string_view` during constant evaluation. Byte assembly is explicitly little-endian, so results are stable across host architectures. |
| `hash_functors.hpp` | Hash-table functors for 64-bit integer keys: `identity_hash` for keys that are already hashes, `moremur_hash` for keys that need `mix_moremur()`. Both are marked avalanching, so `ankerl::unordered_dense` and `boost::unordered` skip their own mixing. |
| `mixers.h` | `mix_moremur()` avalanches one 64-bit integer, useful for finalizing or decorrelating an already assembled key. |
| `rapidhash.hpp` | `rapidhash64()` hashes a byte range with seed 0 or a caller-supplied seed, and `rapidhash64v()` hashes an object's in-memory representation. Wraps `rapidhash.h`, upstream rapidhash V3 with `#pragma once` added and its C++ definitions moved into `rapidhash_detail`. |
| `wheathash.hpp` | `wheathash64()` hashes a byte range with a fixed or caller-supplied seed; `wheathash32()` truncates it to 32 bits, and `wheathash64v()` hashes an object's in-memory representation. |

`rapidhash64v()` and `wheathash64v()` include padding and use the host object representation. Their results may therefore vary with compiler, ABI, byte order, or uninitialized padding; use the byte-range overload with an explicitly serialized representation when hashes must persist or cross process/platform boundaries.
