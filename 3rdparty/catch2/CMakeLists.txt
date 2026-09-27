# 3.20 for C++20 standard support across all three compilers
cmake_minimum_required(VERSION 3.20)

project(catch2_runner LANGUAGES CXX)

# Catch2's implementation compiled once, for every test binary that links it
add_library(catch2_runner STATIC catch_runner.cpp)

target_compile_features(catch2_runner PUBLIC cxx_std_20)
set_target_properties(catch2_runner PROPERTIES CXX_EXTENSIONS OFF)

# Sources include through the library root: "3rdparty/catch2/catch_runner.h"
target_include_directories(catch2_runner PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../..)

if (MSVC)
	target_compile_options(catch2_runner PRIVATE /permissive- /Zc:__cplusplus)
	# power_throttling.hpp includes Windows.h ahead of catch.hpp
	target_compile_definitions(catch2_runner PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()
