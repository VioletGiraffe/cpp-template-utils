#pragma once

template <typename T>
struct type_wrapper {
	using type = T;
};

template <auto v>
struct value_as_type {
	static constexpr auto value = v;
	using type = decltype(value);

#ifdef _MSC_VER
	constexpr operator auto() -> type const {
		return v;
	}
#else
	constexpr operator type() const {
		return v;
	}
#endif
};
