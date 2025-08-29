project(frxml)

file(GLOB SOURCES lib/*.cxx)
file(GLOB_RECURSE HEADERS include/*.h)

add_library(frxml SHARED ${SOURCES} ${HEADERS})
target_compile_options(frxml PUBLIC -fvisibility=hidden)
target_include_directories(frxml PUBLIC include)
target_compile_definitions(frxml PUBLIC cxx_std_17)
