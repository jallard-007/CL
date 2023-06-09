cmake_minimum_required(VERSION 3.10)
find_package(Catch2 3 REQUIRED)
project(main CXX)
set(CMAKE_CXX_STANDARD 17)

add_library(common STATIC ./src/checker/checker.cpp ./src/prettyPrint/prettyPrint.cpp ./src/parser/parser.cpp ./src/nodes.cpp ./src/tokenizer/tokenizer.cpp ./src/token.cpp)

set_target_properties(common PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/out)

add_executable(main ./src/main.cpp)
target_link_libraries(main PRIVATE common)

add_executable(test ./src/tokenizer/test_tokenizer.cpp ./src/parser/test_parser.cpp ./src/prettyPrint/test_prettyPrint.cpp ./src/checker/test_checker.cpp)
target_link_libraries(test PRIVATE common Catch2::Catch2WithMain)

if ( UNIX )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Wall -Wextra -Wpedantic -Werror")
endif()
if ( MSVC )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif()

# -fprofile-arcs -ftest-coverage
# -pg
# gprof main gmon.out | gprof2dot -s | dot -Gdpi=200 -Tpng -o output.png
