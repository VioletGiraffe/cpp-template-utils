## Helper templates for accessing and manipulating template parameter packs

### parameter_pack_helpers.hpp

* `template <typename T, typename... Args> constexpr size_t index_for_type_v` defines the index (position) of *type* T in the `Args...` pack. Results in compilation error if the type is not found.
* `template <size_t index, typename... Args> using type_by_index` defines the *type* at `index` in `Args...`.
* `template <size_t index, typename... Args> constexpr auto value_by_index(Args&&... args) noexcept` returns the *value* at position `index` from `args`.
* `template <typename Functor, class... Args> void apply(Functor&& f, Args&&... args)` iterates the pack `args` and calls functor `f` with each *value* in the pack.
Example: `apply([](auto&& item){std::cout << value << std::endl;}, 0, -1.0f, "text");`
* `template <typename... Args, typename Functor> void for_type(Functor&& f)` calls functor `f` for each *type* in the argument pack. The type is passed using `type_wrapper` helper template defined in `utils/template_magic.hpp`. 
Example: `for_type<int, float, std::string> ([](auto&& t){ using Type = typename decltype(t)::type; });`
