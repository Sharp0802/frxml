project(frxml-test)

file(GLOB SOURCES lib/*.cxx)
file(GLOB_RECURSE HEADERS include/*.h)

add_executable(frxml-test ${SOURCES} ${HEADERS})
target_include_directories(frxml-test PUBLIC include)
target_compile_definitions(frxml-test PUBLIC cxx_std_17)
target_link_libraries(frxml-test PUBLIC benchmark::benchmark frxml pugixml tinyxml2)

file(GLOB SAMPLES samples/*.xml)
foreach (SAMPLE IN LISTS SAMPLES)
    get_filename_component(SAMPLE_NAME ${SAMPLE} NAME)
    add_custom_command(TARGET frxml-test POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${SAMPLE}" "${CMAKE_CURRENT_BINARY_DIR}/${SAMPLE_NAME}"
    )
endforeach ()
