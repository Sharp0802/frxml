cmake_minimum_required(VERSION 3.27)
project(frxml)

set(BENCHMARK_ENABLE_TESTING OFF)

add_subdirectory(dep/benchmark)
add_subdirectory(dep/pugixml)
add_subdirectory(dep/tinyxml2)
add_subdirectory(frxml)
add_subdirectory(frxml-test)
