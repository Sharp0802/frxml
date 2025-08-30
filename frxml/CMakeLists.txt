project(frxml)

file(GLOB_RECURSE HEADERS include/*.h)

add_library(frxml INTERFACE ${HEADERS})
target_include_directories(frxml INTERFACE include)
target_compile_definitions(frxml INTERFACE cxx_std_17)
