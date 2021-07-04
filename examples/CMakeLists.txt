# Example with Vector
add_executable(example_vector EXCLUDE_FROM_ALL example_vector.cpp)
target_link_libraries(example_vector PRIVATE mabe::locked)
set_target_properties(example_vector PROPERTIES CXX_STANDARD 14)

find_package(Threads REQUIRED)
target_link_libraries(example_vector PRIVATE Threads::Threads)