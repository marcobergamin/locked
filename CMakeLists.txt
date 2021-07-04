cmake_minimum_required(VERSION 3.15)
project(locked CXX)

add_library(mabe_locked INTERFACE)
target_include_directories(mabe_locked INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(mabe_locked INTERFACE $<$<COMPILE_FEATURES:cxx_std_17>:MABE_LOCKED_HAS_CXX_STD_17>)
add_library(mabe::locked ALIAS mabe_locked)

add_subdirectory(examples)
enable_testing()
add_subdirectory(test)
