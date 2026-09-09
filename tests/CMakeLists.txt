# 3.20 for C++20 standard support across all three compilers
cmake_minimum_required(VERSION 3.20)

project(cpp-template-utils-tests LANGUAGES CXX)

set(TARGET_NAME tests)

# The library under test is header-only, so there is nothing to build besides this executable.
# Listed in the order of SOURCES in test-app/cpp-template-utils-test-app.pro, which must hold the same files.
# test-app/tracking_allocator.cpp is in neither list: work in progress, deliberately not built.
add_executable(${TARGET_NAME}
	test-app/math_tests.cpp
	test-app/chunked_deque_tests.cpp
	test-app/chunked_deque_benchmarks.cpp
	test-app/callback_caller_tests.cpp
	test-app/flat_map_tests.cpp
	test-app/iterator_helpers_tests.cpp
	test-app/memory_cast_tests.cpp
	test-app/multiindex_tests.cpp
	test-app/odd_sized_integer_tests.cpp
	test-app/regex_helpers_tests.cpp
	test-app/set_operations_tests.cpp
	test-app/static_data_buffer_tests.cpp
	test-app/constexpr_algos_tests.cpp
	test-app/extra_type_traits_tests.cpp
	test-app/parameter_pack_tests.cpp
	test-app/tuple_helpers_tests.cpp
	test-app/wheathash_tests.cpp
)

target_compile_features(${TARGET_NAME} PRIVATE cxx_std_20)
# CONFIG += strict_c++ in the .pro
set_target_properties(${TARGET_NAME} PROPERTIES CXX_EXTENSIONS OFF)

# Test sources include through the library root: "container/flat_map.hpp", "3rdparty/catch2/catch.hpp"
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/..)

# chunked_deque_benchmarks.cpp needs Catch2's benchmarking support
target_compile_definitions(${TARGET_NAME} PRIVATE CATCH_CONFIG_ENABLE_BENCHMARKING)

if (MSVC)
	target_compile_options(${TARGET_NAME} PRIVATE /permissive- /Zc:__cplusplus /MP /W4)
	target_compile_definitions(${TARGET_NAME} PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	target_compile_options(${TARGET_NAME} PRIVATE -Wall)
	target_compile_definitions(${TARGET_NAME} PRIVATE $<$<CONFIG:Debug>:_DEBUG>)
endif()

# $<0:> suppresses the per-configuration subdirectory MSBuild otherwise appends
if (DEFINED OUTPUT_DIR)
	set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/$<0:>)
endif()

enable_testing()
add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
