cmake_minimum_required(VERSION 3.5)

project(loxy)

set(SOURCES
    src/Compiler/Scanner.cc
    src/Compiler/Parser.cc
    src/VM/Chunk.cc
    src/VM/Value.cc
    )

set (CMAKE_CXX_STANDARD 14)

add_executable(loxy ${SOURCES})
target_include_directories(
  loxy PUBLIC
  src/Data
  src/Compiler
  src/VM)

include_directories(src)