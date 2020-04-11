## Helper templates for working specifically with std::tuple

### tuple_helpers.hpp

Defines namespace `tuple`, containing the following templates:
* `constexpr size_t tuple_size_v_omnivorous` is analogous to `std::tuple_size_v` but accepts types with any cv and ref qualifiers.
* `indexForType` returns the index of type `T` in the given tuple; results in compilation error if type not present.
* `visit` implements runtime indexing of a tuple - calls a functor the value at specified index (that need not be known at compile time) as the functor's argument. The argument can be a non-const reference for a non-const tuple, so it can mutate the tuple's items.
* `for_each` calls a functor with each value from the tuple. Supports both `const` and non-`const` tuples. The argument can be a non-const reference for a non-const tuple, so it can mutate the tuple's items.
