win*{
	TEMPLATE = lib
} else {
	TEMPLATE = aux
}

CONFIG += staticlib
CONFIG -= qt

CONFIG += strict_c++ c++14

HEADERS += \
	math/math.hpp \
	container/set_operations.hpp \
	container/iterator_helpers.hpp \
	container/algorithms.hpp \
	utility/on_scope_exit.hpp \
	utility/callback_caller.hpp \
	utility/macro_utils.h \
	container/std_container_helpers.hpp \
	container/ordered_containers.hpp
