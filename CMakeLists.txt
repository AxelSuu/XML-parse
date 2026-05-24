cmake_minimum_required(VERSION 3.15)
project(xml_parser C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    expat
    GIT_REPOSITORY https://github.com/libexpat/libexpat.git
    SOURCE_SUBDIR expat
)
FetchContent_MakeAvailable(expat)

FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
)
FetchContent_MakeAvailable(unity)

# --- Library: all logic except main.c ---
add_library(xml_parser_lib
    src/parser.c
)
target_include_directories(xml_parser_lib PUBLIC
    include
)
target_link_libraries(xml_parser_lib PUBLIC
    expat
)
target_compile_options(xml_parser_lib PRIVATE -Wall -Wextra -Werror)

# --- Main executable ---
add_executable(xml_parser src/main.c)
target_link_libraries(xml_parser PRIVATE xml_parser_lib)

# --- Tests ---
enable_testing()

add_executable(test_parser tests/test_parser.c)
target_link_libraries(test_parser PRIVATE unity xml_parser_lib)
add_test(NAME test_parser COMMAND test_parser)
