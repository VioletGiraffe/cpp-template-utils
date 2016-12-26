win*{
    TEMPLATE = lib
} else {
    TEMPLATE = aux
}

CONFIG += staticlib
CONFIG -= qt

HEADERS += \
    math/math.hpp \
    container/set_operations.hpp \
    container/iterator_helpers.hpp \
    container/algorithms.hpp \
    utility/on_scope_exit.hpp \
    utility/callback_caller.hpp
