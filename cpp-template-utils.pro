win*{
	TEMPLATE = lib
} else {
	TEMPLATE = aux
}

CONFIG += staticlib
CONFIG -= qt
CONFIG -= flat

CONFIG += strict_c++ c++2a

*g++*:QMAKE_CXXFLAGS += -fconcepts -std=c++2a
*msvc*:QMAKE_CXXFLAGS += /Zc:char8_t /JMC


HEADERS += \
	compiler/compiler_warnings_control.h \
	$$files(compiler/*.hpp, false) \
	$$files(container/*.hpp, false) \
	$$files(hash/*.hpp, false) \
	math/math.hpp \
	parameter_pack/parameter_pack_helpers.hpp \
	random/randomnumbergenerator.h \
	regex/regex_helpers.hpp \
	string/string_helpers.hpp \
	tuple/tuple_helpers.hpp \
	$$files(utility/*.hpp, false)
