CONFIG += strict_c++
CONFIG += c++latest

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
	QMAKE_CXXFLAGS += /permissive- /Zc:__cplusplus

	QMAKE_CXXFLAGS += /Zi /FS /MP
	QMAKE_CXXFLAGS += /wd4251
	QMAKE_CXXFLAGS_WARN_ON = /W4

	# MSVC 14.51's code generator faults on chunked_deque_tests.cpp at -O2
	QMAKE_CXXFLAGS_RELEASE -= -O2
	QMAKE_CXXFLAGS_RELEASE += -Od

	DEFINES += WIN32_LEAN_AND_MEAN NOMINMAX
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
	chunked_deque_tests.cpp \
	flat_map_tests.cpp \
	multiindex_tests.cpp \
	odd_sized_integer_tests.cpp \
	static_data_buffer_tests.cpp \
	constexpr_algos_tests.cpp \
	extra_type_traits_tests.cpp \
	parameter_pack_tests.cpp \
	tuple_helpers_tests.cpp
