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
	QMAKE_CXXFLAGS_WARN_ON = -Wall
	QMAKE_CXXFLAGS += -std=c++2a

	Release:DEFINES += NDEBUG=1
	Debug:DEFINES += _DEBUG
}

*g++*:QMAKE_CXXFLAGS += -fconcepts

INCLUDEPATH += $${PWD}/../../

SOURCES += \
	multiindex_tests.cpp \
	tests_main.cpp \
	constexpr_algos_tests.cpp \
	extra_type_traits_tests.cpp \
	parameter_pack_tests.cpp \
	tracking_allocator.cpp \
	tuple_helpers_tests.cpp \
	utils_tests.cpp
