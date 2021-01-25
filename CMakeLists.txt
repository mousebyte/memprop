cmake_minimum_required(VERSION 3.17)

project("memprop")
set(CMAKE_CXX_STANDARD 20)
add_subdirectory(lib/Catch2)
add_executable(tests tests/tests.cpp)
target_include_directories(tests PUBLIC include)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
