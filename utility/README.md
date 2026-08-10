# General utilities

Compile-time, representation, lifetime, and preprocessor helpers that do not belong to a narrower module.

| Header | Facility |
|---|---|
| `aligned_wrapper.hpp` | `Aligned<T, N>` stores a value at explicit alignment; `CacheLinePadded<T>` selects 128-byte alignment on Apple platforms and 64 bytes elsewhere. |
| `callback_caller.hpp` | Non-owning subscriber registry with add/remove and interface-method broadcast. By-value arguments remain intact for every subscriber and are moved only into the final call. |
| `constexpr_algorithms.hpp` | `consteval_for`, fold-based `constexpr_for_fold`/`static_for`, and dispatch from a bounded runtime value to a non-type template argument. |
| `extra_type_traits.hpp` | Traits for trivial byte serialization, cv/ref removal, specialization detection, member-pointer value type, equality comparability, and random-access sortable containers. |
| `integer_literals.hpp` | `_u64`, `_i64`, `_u16`, `_i16`, and `_z` fixed-type integer literal suffixes. |
| `macro_utils.h` | Expansion-aware two/three-token concatenation and stringification macros. |
| `memory_cast.hpp` | `memory_cast` copies bytes into a trivially serializable target from a same-sized value or caller-supplied pointer; `zero_object` clears an object's raw representation. |
| `named_type_wrapper.hpp` | `NamedType<T, Tag>` distinguishes otherwise identical trivial values while retaining implicit conversion to `T`; macros generate line-unique named types. |
| `odd_sized_integer.hpp` | Comparable unsigned integer stored in any native-representation width from one through eight bytes. |
| `on_scope_exit.hpp` | `EXEC_ON_SCOPE_EXIT` creates a non-movable RAII callback whose automatically generated local name is unique per source line. |
| `optional_consteval.hpp` | Optional-like value whose construction and access are restricted to constant evaluation. |
| `static_data_buffer.hpp` | Fixed-capacity byte buffer with logical size, cursor, seek, bounded read/write, direct data access, and range iteration. |
| `template_magic.hpp` | Intentional compile-failure macros, `sfinae` alias, compile-time type printer, and overload-set builder for visitors. |

## Common patterns

```cpp
EXEC_ON_SCOPE_EXIT([&] {
    releaseResource();
});

auto visitor = overload{
    [](int) {},
    [](const std::string&) {}
};
```

`CallbackCaller` stores raw subscriber pointers and does not manage their lifetime. Subscribers must be removed before destruction. `memory_cast`, `zero_object`, and `odd_sized_integer` operate on object representations; use them only when padding, byte order, and representation stability are acceptable for the intended boundary.
