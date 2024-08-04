project(frxml-test)

file(GLOB SOURCES src/*.cxx)
file(GLOB HEADERS *.h)

add_executable(frxml-test ${SOURCES} ${HEADERS})
target_include_directories(frxml-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(frxml-test PUBLIC cxx_std_17)
target_link_libraries(frxml-test PUBLIC frxml)
