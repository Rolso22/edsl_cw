cmake_minimum_required(VERSION 3.22)
project(edsl_cw)

set(CMAKE_CXX_STANDARD 17)

add_executable(edsl_cw main.cpp parser_edsl.cpp parser_edsl.h LrZero.cpp LrZero.h Grammar.cpp Grammar.h LalrOne.cpp LalrOne.h)
