project(frxml-test)

file(GLOB SOURCES lib/*.cxx)
file(GLOB_RECURSE HEADERS include/*.h)

add_executable(frxml-test ${SOURCES} ${HEADERS})
target_include_directories(frxml-test PUBLIC include)
target_compile_definitions(frxml-test PUBLIC cxx_std_17)
target_link_libraries(frxml-test PUBLIC benchmark::benchmark frxml pugixml)

add_custom_command(TARGET frxml-test POST_BUILD
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/samples/0.xml" "0.xml"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/samples/1.xml" "1.xml"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/samples/2.xml" "2.xml"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/samples/3.xml" "3.xml"
)
