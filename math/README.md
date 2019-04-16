## A collection of arithmetic convenience functions

### math.hpp

Defines a number of template functions for mathematical and arithmetical operations in the `Math` namespace.

* `round` and `floor` functions that accept a second parameter - the number of digits after the decimal point to round or floor to.
* `round`, `floor` and `ceil` functions that accept an extra template parameter for the output type which can be integer or floating-point.
* `abs` function suitable both for integer and floating-point values. For integer values, it takes in to account that the opposite value for `std::numeric_limits<T>::min()` is not representable in the same type, so it truncates it to `::max()`.
* `clamp(T min, T value, T max)` function template.
* `signum` function template (returns -1 for negative value, 0 for zero and 1 for positive).
* `squared` function template that accepts an output type that can be larger than the inputtype  to fit the result reliably.
* `bool isInRange(const T value, const T lowerBound, const T upperBound)` function template.
* `arithmeticMean`, `geometricMean`: function templates that take a parameter pack and return the mean value of the values in the pack. Can be used to conveniently calculate the mean of 3-4-5 values.
