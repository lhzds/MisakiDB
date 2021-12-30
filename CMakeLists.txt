cmake_minimum_required(VERSION 3.20)
project(MisakiDB)

set(CMAKE_CXX_STANDARD 20)
add_compile_options(-fconcepts)

file(GLOB_RECURSE SOURCES "src/*.cpp")
get_filename_component(MAIN_CPP src/main.cpp ABSOLUTE)
get_filename_component(SERVER_CPP src/server.cpp ABSOLUTE)
set(SERVER ${MAIN_CPP} ${SERVER_CPP})
list(REMOVE_ITEM SOURCES ${SERVER})

add_library(MisakiDB ${SOURCES})
add_executable(Server ${SERVER})
target_link_libraries(Server MisakiDB ws2_32)

include_directories("src/include")
add_subdirectory(test)
