cmake_minimum_required(VERSION 3.15)
project(locked CXX)
enable_testing()

set(CMAKE_CXX_STANDARD 11)

add_library(mabe_locked INTERFACE)
target_include_directories(mabe_locked INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(mabe::locked ALIAS mabe_locked)

add_subdirectory(examples)

find_package(GTest)
if (GTest_FOUND)
    add_executable(locked_test test/locked_test.cpp)
    target_link_libraries(locked_test mabe::locked GTest::Main gmock)
    target_compile_definitions(mabe_locked INTERFACE MABE_LOCKED_TESTING)
    gtest_add_tests(TARGET locked_test)
    set_target_properties(locked_test PROPERTIES CXX_STANDARD 17)
endif()
