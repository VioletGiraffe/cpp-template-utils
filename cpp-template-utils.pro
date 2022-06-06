win*{
	TEMPLATE = lib
} else {
	TEMPLATE = aux
}

CONFIG += staticlib
CONFIG -= qt
CONFIG -= flat

CONFIG += strict_c++ c++2a

*g++*:QMAKE_CXXFLAGS += -fconcepts -std=c++2a
*msvc*:QMAKE_CXXFLAGS += /Zc:char8_t /JMC


HEADERS += \
	compiler/compiler_warnings_control.h \
	container/multi_index.hpp \
	container/multimap_helpers.hpp \
	container/tracking_allocator.hpp \
	container/vector2d.hpp \
	math/math.hpp \
	container/set_operations.hpp \
	container/iterator_helpers.hpp \
	container/algorithms.hpp \
	string/string_helpers.hpp \
	utility/data_buffer.hpp \
	utility/extra_type_traits.hpp \
	utility/memory_cast.hpp \
	utility/named_type_wrapper.hpp \
	utility/on_scope_exit.hpp \
	utility/callback_caller.hpp \
	utility/macro_utils.h \
	utility/constexpr_algorithms.hpp \
	container/std_container_helpers.hpp \
	container/ordered_containers.hpp \
	container/string_helpers.hpp \
	tuple/tuple_helpers.hpp \
	parameter_pack/parameter_pack_helpers.hpp \
	random/randomnumbergenerator.h \
	regex/regex_helpers.hpp \
	utility/optional_consteval.hpp \
	utility/template_magic.hpp \
	utility/integer_literals.hpp
