cmake_minimum_required(VERSION 3.20)
project(passman CXX)

add_executable(passman)
target_compile_features(passman PRIVATE cxx_std_23)
target_link_libraries(passman PRIVATE readline)

add_subdirectory(src)
