win*{
	TEMPLATE = lib
} else {
	TEMPLATE = aux
}

CONFIG += staticlib
CONFIG -= qt

CONFIG += strict_c++ c++17

HEADERS += \
	math/math.hpp \
	container/set_operations.hpp \
	container/iterator_helpers.hpp \
	container/algorithms.hpp \
	utility/on_scope_exit.hpp \
	utility/callback_caller.hpp \
	utility/macro_utils.h \
	utility/constexpr_algorithms.hpp \
	container/std_container_helpers.hpp \
	container/ordered_containers.hpp \
	container/string_helpers.hpp \
	random/crandomnumbergenerator.h \
	tuple/tuple_helpers.hpp
