cmake_minimum_required(VERSION 3.5)

project(loxy)

set(SOURCES
    src/chunk.cc
    src/compiler.cc
    src/memory.cc
    src/table.cc
    src/value.cc)

set (CMAKE_CXX_STANDARD 11)

add_executable(loxy ${SOURCES})
target_include_directories(
  loxy PUBLIC
  src)