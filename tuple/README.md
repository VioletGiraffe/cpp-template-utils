## Helper templates for working specifically with std::tuple

### tuple_helpers.hpp

Defines namespace `tuple`, containing the following templates:
* `for_each` calls a functor with each value from the tuple.
* `indexForType` returns the index of type `T` in the given tuple; results in compilation error if type not present.
* `visit` implements runtime indexing of a tuple - gets the value at specified index that need not be known at compile time.
