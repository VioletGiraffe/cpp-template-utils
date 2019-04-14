##A collection of algorithms and augmentations for STL-compatible containers

#algorithms.hpp

Defines two functions in `ContainerAlgorithms` namespace that are constructed upon the standard algorithms:
* `void erase_all_occurrences(ContainerType& container, const ArgumentType& item)` deletes every occurrence of the `item` in `container` using the remove-erase idiom.
* `void void erase_if(ContainerType& container, std::function<bool(const ItemType&)>` deletes every occurrence of the `item` in `container` using the remove-erase idiom.

#iterator_helpers.hpp

Defines two classes `const_forward_iterator_wrapper` and `forward_iterator_wrapper` that encapsulate an std (or std-compatible) iterator together with a reference to the container this iterator points to. This allows using these iterators as any other normal iterator while also being able to get the parent container from them.

#ordered_containers.hpp

Defines `ordered_container` class that wraps an STL-compatible container. It is intended for use with containers that aren't sorted by nature (e. g. vector or list as opposed to map or set), and provides three extra methods: `sort()`, `find(value)` and `insert_into_sorted(value)`. The `find` and `insert_into_sorted` methods require that container is sorted, and for such a container they provide optimized implementation using `std::lower_bound`. The `insert_into_sorted` method returns `std::pair<iterator, bool>` similar to the standard ordered containers.

#set_operations.hpp

Defines a number of algorithms on containers in `SetOperations` namespace:
* `OrderedSetType longestCommonStart(SupersetType<OrderedSetType> const & superset)` takes a set of ordered containers and returns the longest common starting sequence of items between all of these ordered containers.
Example 1: `longestCommonStart(std::vector{std::vector<int>{1, 2, 3, 4, 5}, std::vector<int>{1, 2, 3, 10, 20}})` -> `std::vector{1, 2}`
Example 2: `longestCommonStart(std::vector{std::string("Hello"), std::string("Heat"), std::string("Home")})` -> `std::string("H")`
* `template <class ContainerType> ContainerType uniqueElements(const ContainerType& c)` returns only the unique items from `c`. This function is stable (item order is preserved). Has no-op overloads for `set` and `map` which may only contain unique items by definition.
* `setTheoreticDifference` takes two containers `a` and `b` and an optional comparator, and returns a container of all the elements from `a` that are not in `b`.
Example: `setTheoreticDifference<std::list>(std::vector<int> {1, 2, 3}, std::deque<int> {3, 1})` -> `std::list<int> {2}`
It is assumed that the containers are unordered because for ordered containers `std::set_difference` can be called directly.
* `calculateDiff` takes two containers `a` and `b` and an optional template argument specifying the output container type. It returns the following structure:
`template <class OutputContainerType>
struct Diff
{
	OutputContainerType common_elements;
	OutputContainerType elements_from_a_not_in_b;
	OutputContainerType elements_from_b_not_in_a;
};`

#std_container_helpers.hpp

Defines two functions that behave differently depending on what container they're called with:
* `void add_item(Container& container, const ItemType& item)` calls `push_back(item)` for containers that have push_back (ordered containers), and `insert(item)` for other (unordered) containers.
* `auto container_aware_find(Container& container, const ItemType& item)` calls `container.find(item)` for containers that have a member function `find`, calls `std::find` otherwise.

#string_helpers.hpp

Defines `bool operator==(const std::string str, const char ch)` and `bool operator==(const char ch, const std::string str)` for comparing a string with to a single character.