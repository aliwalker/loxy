#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <memory>
#include "common.h"
#include "vm.h"

using namespace loxy;

static void repl(LoxyVM &vm) {
  std::string line;
  
  while (true) {
    std::cout << "> ";

    if (std::getline(std::cin, line).eof()) {
      std::cout << std::endl;
      break;
    }

    vm.interpret(line.c_str(), "main");
  }
}

static std::string readFile(const char *path) {
  std::ifstream t(path);
  if (t.bad()) {
    std::cerr << std::string("Unable to find file '") + std::string(path) << "'\n";
    exit(80);
  }
  std::string source((std::istreambuf_iterator<char>(t)),
                      std::istreambuf_iterator<char>());
  return source;
}

static void runFile(LoxyVM &vm, const char *path) {
  auto source = readFile(path);
  InterpretResult result = vm.interpret(source.c_str(), "main");
  if (result == InterpretResult::Compile_Error) exit(65);
  if (result == InterpretResult::Runtime_Error) exit(70);
}

int main(int argc, const char *argv[]) {
  LoxyVM vm;

  if (argc == 1) {
    repl(vm);
  } else if (argc == 2) {
    runFile(vm, argv[1]);
  } else {
    std::cerr << "Usage: loxy" << std::endl;
  }
  
  return 0;
}