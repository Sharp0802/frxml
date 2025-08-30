project(frxml-test)

file(GLOB SOURCES lib/*.cxx)
file(GLOB_RECURSE HEADERS include/*.h)

add_executable(frxml-test ${SOURCES} ${HEADERS})
target_include_directories(frxml-test PUBLIC include)
target_compile_definitions(frxml-test PUBLIC cxx_std_17)
target_link_libraries(frxml-test PUBLIC ubench)
