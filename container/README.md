# Container utilities

Algorithms, adapters, and containers for STL-compatible code.

## Facilities

| Header | Facility |
|---|---|
| `algorithms.hpp` | `ContainerAlgorithms::erase_if()` applies the remove/erase idiom to sequence containers. |
| `flat_map.hpp` | Vector-backed sorted `flat_map` and `flat_set` with heterogeneous lookup, random-access iteration, ordinary insertion/erasure, sorted-range merging, and batched unsorted insertion. |
| `iterator_helpers.hpp` | `const_forward_iterator_wrapper` retains both an iterator and its parent container, allowing validity and end checks; factories provide wrapped `cbegin`/`cend`. |
| `multi_index.hpp` | `MultiIndexSet` owns values uniquely by one member and maintains a non-unique secondary-member index with exact and range lookup. |
| `multimap_helpers.hpp` | `multimap_value_iterator` adapts a multimap iterator to expose only its mapped value while retaining access to the native iterator. |
| `ordered_containers.hpp` | `ordered_container` adds explicit sorting, lower-bound lookup, and unique sorted insertion to sequence containers. |
| `set_operations.hpp` | Common-prefix, deduplication, difference, three-way diff, and order-insensitive equality algorithms. |
| `std_container_helpers.hpp` | Begin/end pair macros, transparent `std::set`, and helpers selecting `push_back`/`insert` or member/linear lookup according to container capabilities. |
| `tracking_allocator.hpp` | Standard allocator wrapper that reports the bytes currently allocated through that allocator instance. |
| `vector2d.hpp` | Rectangular `vector<vector<T>>` helper with two-dimensional resize, row fill, width, and height. |

## Flat associative containers

`flat_map` stores keys and mapped values in separate vectors and exposes pair-like proxy iterators with `first`/`second` and `key()`/`value()` access. Map dereference returns its proxy by value: `auto entry` and `const auto& entry` work in range loops, but `auto& entry` does not. Keys remain immutable, so read-only standard algorithms and construction of ordinary pair containers work while algorithms that reorder entries are intentionally ill-formed.

Both flat containers support ordinary insertion, merging a sorted range, `append_sorted_unique()`, and batched unsorted appends. Between `begin_batch()` and `end_batch()`, ordered operations and iteration are invalid. Finalization sorts only the appended tail and merges it with the existing prefix. Existing entries win conflicts with a batch, and the first batch entry wins duplicates within that batch.

Key equality uses `operator==` when the compared types provide it and comparator equivalence otherwise. When both are available, they must describe the same equivalence relation.

## Set operations

- `longestCommonStart()` returns the longest shared prefix of a container of ordered containers: `std::vector<std::string>{"Hello", "Heat", "Home"}` produces `"H"`.
- `uniqueElements<ItemOrder>()` removes duplicates, optionally retaining the first or last occurrence order. The `std::set` overload is a no-op reference return.
- `setTheoreticDifference<OutputContainer>()` sorts copies of two unordered inputs and returns the elements present only in the first.
- `calculateDiff()` returns `common_elements`, `elements_from_a_not_in_b`, and `elements_from_b_not_in_a`.
- `is_equal_sets()` compares compatible containers without regard to order; it sorts non-`std::set` inputs in place.
