# C++ template utilities

Header-only C++20 utility library covering generic containers, compile-time programming, hashing, math, and low-level helpers. Add the repository root to the include path; no library target needs to be linked.

## Facilities

### Compiler and language helpers

| Header | Facility |
|---|---|
| `compiler/compiler_warnings_control.h` | Portable macros for saving/restoring diagnostic state, suppressing all or selected MSVC/Clang/GCC warnings, and marking variables unused. |
| `lang/utils.hpp` | Short move alias plus macros that declare a type non-copyable or fully non-movable. |
| `utility/macro_utils.h` | Expansion-aware token concatenation and stringification macros. |
| `utility/template_magic.hpp` | Intentional compile-failure macros, an `sfinae` alias, compile-time type printer, and overload-set builder for visitors. |

### Containers and algorithms

| Header | Facility |
|---|---|
| `container/algorithms.hpp` | Remove/erase-style `ContainerAlgorithms::erase_if()` for sequence containers. |
| `container/flat_map.hpp` | Vector-backed sorted `flat_map` and `flat_set` with heterogeneous lookup, random-access iteration, ordinary insert/erase, sorted-range merge, and efficient unsorted batch append/finalize. `flat_map` stores keys and values in separate vectors and exposes pair-like proxy references. |
| `container/iterator_helpers.hpp` | Const forward-iterator wrapper that retains its parent container and can report validity or end reached, plus `cbegin`/`cend` factories. |
| `container/multi_index.hpp`, `container/multimap_helpers.hpp` | `MultiIndexSet`, owning values uniquely by a primary member and indexing them non-uniquely by a secondary member, with primary, exact-secondary, and secondary-range lookup; includes mapped-value iterator adaptation. |
| `container/ordered_containers.hpp` | Adapter for explicitly sorted sequence containers, adding lower-bound `find`, unique sorted insertion, and sorting. |
| `container/set_operations.hpp` | Longest common prefix, stable/configurable deduplication, unordered set difference, three-way diff, and order-insensitive equality across compatible containers (sorting non-set inputs in place). |
| `container/std_container_helpers.hpp` | Begin/end pair macros, transparent `std::set` alias, and helpers that select `push_back` versus `insert` and member `find` versus `std::find`. |
| `container/tracking_allocator.hpp` | Standard allocator wrapper that reports bytes currently allocated through that allocator instance. |
| `container/vector2d.hpp` | Rectangular `vector<vector<T>>` convenience type with two-dimensional resize, row fill, width, and height. |

### Compile-time and type utilities

| Header | Facility |
|---|---|
| `parameter_pack/parameter_pack_helpers.hpp` | Pack type/value lookup, occurrence tests/counts, value/type iteration, and a `type_pack` supporting indexing, append, tuple conversion, and template construction. |
| `tuple/tuple_helpers.hpp` | cv/ref-tolerant tuple size, type index lookup, compile-time size, and value-preserving `for_each`. |
| `utility/constexpr_algorithms.hpp` | `consteval` and fold-based compile-time loops plus bounded dispatch from a runtime value to a non-type template argument. |
| `utility/extra_type_traits.hpp` | Traits for trivial byte serialization, cv/ref removal, template specialization detection, member-pointer value type, equality comparability, and sortable containers. |
| `utility/optional_consteval.hpp` | Small optional-like value restricted to constant evaluation. |

### Hashing, math, and random numbers

| Header | Facility |
|---|---|
| `hash/hash_consteval.hpp` | Seedable compile-time MurmurHash3 x86-32 for string views. |
| `hash/mixers.h` | `mix_moremur()`, a fast 64-bit integer finalizer/mixer. |
| `hash/wheathash.hpp` | Seeded or fixed-seed 32/64-bit wheathash for byte ranges and whole object representations. |
| `math/math.hpp` | Typed rounding/floor/ceil, safe integral and floating absolute value, clamp/sign/square/range helpers, arithmetic/geometric means, powers of two, range reduction, and precomputed fast 32-bit modulo. |
| `random/randomnumbergenerator.h` | Deterministically seeded generator-plus-distribution wrapper, thread-local static ranged RNG facade, and printable-character specialization. |

### Strings and regular expressions

| Header | Facility |
|---|---|
| `regex/regex_helpers.hpp` | Callback-driven regex replacement and replacement of a selected capture within one match. |
| `string/string_helpers.hpp` | Single-character string comparison and stream-like/numeric append operators for standard strings. |

### Storage, representation, and lifetime

| Header | Facility |
|---|---|
| `utility/aligned_wrapper.hpp` | Explicitly aligned value wrapper and platform-sized `CacheLinePadded` alias. |
| `utility/callback_caller.hpp` | Non-owning multi-subscriber registry that invokes an interface method for every subscriber without prematurely moving by-value arguments. |
| `utility/integer_literals.hpp` | `_u64`, `_i64`, `_u16`, `_i16`, and `_z` fixed-type integer literals. |
| `utility/memory_cast.hpp` | Checked byte-copy conversion between trivially serializable representations and whole-object zeroing. |
| `utility/named_type_wrapper.hpp` | Lightweight strong-ish named wrapper for trivial values, with macros for call-site-unique named types. |
| `utility/odd_sized_integer.hpp` | Comparable unsigned integer stored in any byte width from 1 through 8. |
| `utility/on_scope_exit.hpp` | `EXEC_ON_SCOPE_EXIT`, an RAII scope-exit callback with an automatically unique local name. |
| `utility/static_data_buffer.hpp` | Fixed-capacity byte buffer with logical size, cursor, seek, bounded read/write, direct data access, and range iteration. |
