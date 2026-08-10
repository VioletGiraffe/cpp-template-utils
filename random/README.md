# Random-number utilities

`randomnumbergenerator.h` combines a standard random engine and distribution behind a small integral interface.

| Facility | Description |
|---|---|
| `RandomNumberGenerator<IntType, DistributionT, GeneratorT>` | Owns one generator and distribution and returns the next value through `rand()`. Defaults to `std::mt19937_64`, `std::uniform_int_distribution`, the full `IntType` range, and deterministic seed `0`. |
| `RNG<IntType, minValue, maxValue, ...>` | Static `next()` facade backed by one thread-local generator per specialization. Each thread starts from seed `0`. |
| `RandomChar` | `RNG<int16_t, 33, 126>` specialization for printable ASCII code points. |

Supply a varying seed to `RandomNumberGenerator` when reproducible output is not desired. `RNG` intentionally exposes no reseeding API.
