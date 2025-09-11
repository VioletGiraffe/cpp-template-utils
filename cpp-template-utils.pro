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
	$$files(compiler/*.h*, false) \
	$$files(container/*.hpp, false) \
	$$files(hash/*.hpp, false) \
	$$files(lang/*.hpp, false) \
	$$files(math/*.hpp, false) \
	$$files(parameter_pack/*.hpp, false) \
	$$files(random/*.hpp, false) \
	$$files(regex/*.hpp, false) \
	$$files(string/*.hpp, false) \
	$$files(tuple/*.hpp, false) \
	$$files(utility/*.h*, false)
