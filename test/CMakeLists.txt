project(frxml-test)

file(GLOB SOURCES src/*.cxx)
file(GLOB HEADERS *.h)

find_package(LibXml2 REQUIRED)
find_package(PugiXML REQUIRED)

add_executable(frxml-test ${SOURCES} ${HEADERS})
target_compile_options(frxml-test PUBLIC -pg)
target_include_directories(frxml-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${LIBXML2_INCLUDE_DIRS})
target_compile_definitions(frxml-test PUBLIC cxx_std_17)
target_link_libraries(frxml-test PUBLIC frxml ${LIBXML2_LIBRARY} pugixml::pugixml)
