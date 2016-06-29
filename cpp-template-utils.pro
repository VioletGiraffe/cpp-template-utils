win*{
    TEMPLATE = lib
} else {
    TEMPLATE = aux
}

CONFIG += staticlib
CONFIG -= qt

HEADERS += \
    container/algorithms.h \
    container/set_operations.h \
    container/iterator_helpers.h
