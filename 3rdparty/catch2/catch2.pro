# Catch2's implementation compiled once, for every test binary of a repository that links it; consumers include catch2.pri

TEMPLATE = lib
TARGET = catch2_runner
CONFIG += staticlib
CONFIG -= qt

CONFIG += strict_c++

mac* | linux* | freebsd {
	CONFIG(release, debug|release):CONFIG *= Release optimize_full
	CONFIG(debug, debug|release):CONFIG *= Debug
}

# The parent repository's settings where this repository is a submodule: the test binaries linking this library use them
exists(../../../global.pri) {
	include(../../../global.pri)
} else {
	CONFIG += c++2b
}

# power_throttling.hpp includes Windows.h ahead of catch.hpp. Set here: a parent's global.pri need not define these.
win*:DEFINES += WIN32_LEAN_AND_MEAN NOMINMAX

# global.pri's LTO removed: the test binaries link without it
# MSVC keeps /GL: the tests linking this library link with LTCG
QMAKE_CFLAGS   -= -flto=auto -ffat-lto-objects -flto=thin
QMAKE_CXXFLAGS -= -flto=auto -ffat-lto-objects -flto=thin

Release:OUTPUT_DIR=release
Debug:OUTPUT_DIR=debug

DESTDIR = ../../../bin/$${OUTPUT_DIR}
OBJECTS_DIR = ../../../build/$${OUTPUT_DIR}/$${TARGET}

INCLUDEPATH += ../..

HEADERS += \
	catch.hpp \
	catch_runner.h \
	test_main.hpp

SOURCES += \
	catch_runner.cpp
