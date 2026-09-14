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
	QMAKE_CXXFLAGS += /std:c++latest /permissive- /Zc:__cplusplus /utf-8

	QMAKE_CXXFLAGS += /Zi /FS /MP
	QMAKE_CXXFLAGS += /wd4251
	QMAKE_CXXFLAGS_WARN_ON = /W4

	DEFINES += WIN32_LEAN_AND_MEAN NOMINMAX

	QMAKE_LFLAGS += /DEBUG
	Release:QMAKE_CXXFLAGS += /GL
	Release:QMAKE_LFLAGS += /OPT:REF /OPT:ICF /LTCG:INCREMENTAL
}

linux*|mac*|freebsd{
	Release:DEFINES += NDEBUG=1
	Debug:DEFINES += _DEBUG
}

linux*:Release {
	QMAKE_CXXFLAGS += -flto=auto
	QMAKE_LFLAGS   += -flto=auto
}

mac*:Release {
	QMAKE_CXXFLAGS += -flto=thin
	QMAKE_LFLAGS   += -flto=thin
}

*g++*:QMAKE_CXXFLAGS += -fconcepts

DEFINES += CATCH_CONFIG_ENABLE_BENCHMARKING

INCLUDEPATH += $${PWD}/../../

# ../CMakeLists.txt builds the same list and must be updated alongside this one
SOURCES += \
	main.cpp \
	math_tests.cpp \
	chunked_deque_tests.cpp \
	chunked_deque_benchmarks.cpp \
	callback_caller_tests.cpp \
	flat_map_tests.cpp \
	iterator_helpers_tests.cpp \
	memory_cast_tests.cpp \
	multiindex_tests.cpp \
	odd_sized_integer_tests.cpp \
	regex_helpers_tests.cpp \
	set_operations_tests.cpp \
	static_data_buffer_tests.cpp \
	constexpr_algos_tests.cpp \
	extra_type_traits_tests.cpp \
	parameter_pack_tests.cpp \
	tuple_helpers_tests.cpp \
	wheathash_tests.cpp
