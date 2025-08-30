project(frxml)

file(GLOB_RECURSE HEADERS include/*.h)

add_library(frxml INTERFACE ${HEADERS})
target_include_directories(frxml INTERFACE include)
target_compile_definitions(frxml INTERFACE cxx_std_17)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(frxml INTERFACE _DEBUG)
endif ()
