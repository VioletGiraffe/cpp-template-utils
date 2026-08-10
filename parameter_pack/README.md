# Parameter-pack utilities

`parameter_pack_helpers.hpp` provides compile-time type lookup and iteration in namespace `pack`, plus the `type_pack` facade.

| Facility | Description |
|---|---|
| `type_by_index<I, Args...>` | Type at index `I`. |
| `index_for_type<T, Args...>()` | Optional compile-time index of the first `T`; `index_for_type_v` is the strict form that fails compilation when absent. |
| `has_type_v<T, Args...>`, `type_count<T, Args...>()` | Tests for a type and counts its occurrences. |
| `first_type<Args...>` | First type in a pack. |
| `value_by_index<I>(args...)` | Value at index `I`, selected during constant evaluation. |
| `for_value(f, args...)` | Calls `f(value)` for every value in order. |
| `for_type<Args...>(f)` | Calls `f.template operator()<T>()` once for every type. |
| `type_pack<Args...>` | Exposes pack size and indexed types, finds type indices, appends types, constructs another variadic template, or converts the pack to `std::tuple`. |

```cpp
pack::for_value([](const auto& value) { std::cout << value << '\n'; }, 0, -1.0f, "text");

pack::for_type<int, float, std::string>([]<typename T> {
    static_assert(sizeof(T) > 0);
});
```
