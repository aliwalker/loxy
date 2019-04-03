cmake_minimum_required(VERSION 3.5)

project(loxy)

set(SOURCES
    src/chunk.cc
    src/compiler.cc
    src/value.cc
    src/vm.cc)

set (CMAKE_CXX_STANDARD 14)

add_executable(loxy ${SOURCES})
target_include_directories(
  loxy PUBLIC
  src/chunk.h
  src/common.h
  src/compiler.h
  src/memory.h
  src/opcode.h
  src/value.h
  src/vm.h)