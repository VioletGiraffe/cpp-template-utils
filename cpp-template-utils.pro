win*{
	TEMPLATE = lib
} else {
	TEMPLATE = aux
}

CONFIG += staticlib
CONFIG -= qt

CONFIG += strict_c++ c++1z

HEADERS += \
	container/multi_index.hpp \
	container/multimap_helpers.hpp \
	math/math.hpp \
	container/set_operations.hpp \
	container/iterator_helpers.hpp \
	container/algorithms.hpp \
	utility/extra_type_traits.hpp \
	utility/memory_cast.hpp \
	utility/on_scope_exit.hpp \
	utility/callback_caller.hpp \
	utility/macro_utils.h \
	utility/constexpr_algorithms.hpp \
	container/std_container_helpers.hpp \
	container/ordered_containers.hpp \
	container/string_helpers.hpp \
	tuple/tuple_helpers.hpp \
	parameter_pack/parameter_pack_helpers.hpp \
	utility/template_magic.hpp \
	utility/integer_literals.hpp \
	random/randomnumbergenerator.h
