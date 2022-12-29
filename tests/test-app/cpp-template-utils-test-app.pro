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
	main.cpp \
	math_tests.cpp \
	multiindex_tests.cpp \
	odd_sized_integer_tests.cpp \
	static_data_buffer_tests.cpp \
	constexpr_algos_tests.cpp \
	extra_type_traits_tests.cpp \
	parameter_pack_tests.cpp \
	tuple_helpers_tests.cpp
