# Tuple utilities

`tuple_helpers.hpp` provides compile-time inspection and value iteration in namespace `tuple`.

| Facility | Description |
|---|---|
| `tuple_size_v_omnivorous<Tuple>` | `std::tuple_size_v` after removing reference qualifiers, accepting const and reference forms. |
| `indexForType<T>(tuple)` | Compile-time index of `T`; fails compilation when the type is absent. |
| `size(tuple)` | Compile-time tuple element count. |
| `for_each(tuple, f)` | Calls `f` with every element in order, preserving tuple and element cv/ref qualifiers so a mutable tuple can be modified. |

```cpp
std::tuple values{1, 2.0};
tuple::for_each(values, []<typename T>(T& value) { value += value; });
```
