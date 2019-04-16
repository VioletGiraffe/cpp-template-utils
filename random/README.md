## Helper class for wrapping generator and distribution inside a single class.

### randomnumbergenerator.h

 I find it annoying to have to explicitly declare both the generator and the distribution, so I wrapped it up in a class. Templated on value type, distribution type and generator type; has reasonable default distribution `std::uniform_int_distribution` and generator `std::mt19937_64`.
 