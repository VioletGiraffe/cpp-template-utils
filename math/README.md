# Math utilities

`math.hpp` provides small arithmetic templates in namespace `Math`.

| Facility | Description |
|---|---|
| `round`, `floor`, `ceil` | Decimal-place rounding/flooring and typed numeric conversions for integral and floating-point inputs. |
| `abs` | Integral and floating absolute value; the unrepresentable magnitude of the signed minimum is saturated to the type's maximum. |
| `clamp`, `signum`, `squared`, `isInRange` | Bounds, sign, optionally widened square, and inclusive range helpers. |
| `arithmeticMean`, `geometricMean` | Mean of a short value parameter pack, returned as the explicitly selected result type. |
| `pow2` | Computes powers of two for positive exponents. |
| `reduce` | Maps a 32-bit value into `[0, range)` using the high half of a multiply. |
| `FastMod32` | Precomputes a non-zero divisor's reciprocal data so repeated 32-bit modulo operations avoid division. |
