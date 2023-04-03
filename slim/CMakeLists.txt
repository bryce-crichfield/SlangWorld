project(slim)

cmake_minimum_required(VERSION 3.17)
set(CMAKE_C_COMPILER "clang")

include_directories(include)
file(GLOB_RECURSE SLIM "source/*.c")

add_executable(exe ${SLIM})
target_link_libraries(exe m)

set_target_properties(exe PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)