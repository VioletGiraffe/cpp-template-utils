## Various convenience functions and macros that are hard to categorize.

### macro_utils.h

C++ preprocessor tricks.
* `CONCAT_EXPANDED_ARGUMENTS_2`, `CONCAT_EXPANDED_ARGUMENTS_3` concatenates two or three arguments together. The trick is that any preprocessor definitions in the arguments will get expanded. Naive concatenation (`a##b`) does not expand macros.
* `STRINGIFY_EXPANDED_ARGUMENT` expands the argument, then stringifies it. Naive stringification (`#a`) does not expand macros.

### callback_caller.hpp

A helper class template for organizing and implementing callbacks (aka listeners, observers). Templated on the interface type, the class supports multiple subscribers and offers the following methods:
* `addSubscriber(Interface* instance)`
* `removeSubscriber(Interface* instance)`
* `template <typename MethodPointer, typename ...Args> void invokeCallback(MethodPointer methodPtr, Args... args)` where MethodPointer is a method of the interface.

### constexpr_algorithms.hpp

A header for storing algorithm-like constexpr snippets:

* `template <int First, int Last, typename Functor> constexpr void static_for(Functor&& f)` calls the functor `f` with each value from the specified integer range. It's like a for loop, but compile time!

### integer_literals.hpp

A collection of custom literal suffixes:

* `_u64` literal results in a value of type `uint64_t`. Example: `uint64_t v = 153_u64;`

### on_scope_exit.hpp

Provides a RAII class that takes a functor in its constructor (e. g. a lambda or `std::function`) and calls it in the destructor. Thus, the code gets called on scope exit. useful for cleanup tasks and whatnot.
The `EXEC_ON_SCOPE_EXIT` macro will make sure each instance gets its own unique variable name with zero manual work required... As long as you don't stuff more than one executor on a single line.

### template_magic.hpp

Just what it sounds like - a bunch of templates that come in handy during template programming.

* `type_wrapper<T>` would perhaps be better named *type_as_value*. The only thing it does is define member typename `using type = T;`. The purpose is to pass the lightweight instance of `type_wrapper` by value and extract the original type from it. Most necessary for passing type information into template lambdas (`[](auto&& t){}`).

* `value_as_type` is the opposite - it turns value into type through use of non-type template parameters.

* `remove_cv_and_reference_t` combines `std::remove_cv_t` and `std::remove_reference_t` in the right order (which does matter).