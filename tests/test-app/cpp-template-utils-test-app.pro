CONFIG += strict_c++ c++2a
CONFIG -= qt

TEMPLATE = app
CONFIG += console
TARGET = tests
DESTDIR = $${PWD}/../bin

mac* | linux* | freebsd {
	CONFIG(release, debug|release):CONFIG *= Release optimize_full
	CONFIG(debug, debug|release):CONFIG *= Debug
}

win*{
	QMAKE_CXXFLAGS += /std:c++latest /permissive- /Zc:__cplusplus

	QMAKE_CXXFLAGS += /MP /Zi /FS
	QMAKE_CXXFLAGS += /wd4251
	QMAKE_CXXFLAGS_WARN_ON = /W4
}

linux*|mac*{
	QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-c++11-extensions -Wno-local-type-template-args -Wno-deprecated-register
	QMAKE_CXXFLAGS += -std=c++2a

	Release:DEFINES += NDEBUG=1
	Debug:DEFINES += _DEBUG
}

*g++*:QMAKE_CXXFLAGS += -fconcepts

INCLUDEPATH += $${PWD}/../../


HEADERS += \
	constexpr_algos_tests.hpp \
	container_tests.hpp \
	extra_type_traits_tests.hpp \
	parameter_pack_tests.hpp \
	tuple_helpers_tests.hpp \
	utils_tests.hpp

SOURCES += tests_main.cpp
