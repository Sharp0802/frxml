project(frxml)

file(GLOB SOURCES src/*.cxx)
file(GLOB_RECURSE HEADERS *.h)

add_library(frxml STATIC ${SOURCES} ${HEADERS})
target_include_directories(frxml PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/internal)
target_include_directories(frxml PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(frxml INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(frxml PUBLIC cxx_std_17)
