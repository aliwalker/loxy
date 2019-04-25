#include <stdio.h>
#include "Common.h"
#include "VM/VM.h"

using namespace loxy;

static char *readFile(const char *f) {
  FILE *file = fopen(f, "rb");

  if (file == NULL) {
    fprintf(stderr, "Could not open file \"%s\".\n", f);
    exit(74);
  }

  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  char* buffer = (char*)malloc(fileSize + 1);

  if (buffer == NULL) {
    fprintf(stderr, "Not enough memory to read \"%s\".\n", f);
    exit(74);
  }

  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
  if (bytesRead < fileSize) {
    fprintf(stderr, "Could not read file \"%s\".\n", f);
    exit(74);
  }

  buffer[bytesRead] = '\0';
  fclose(file);
  return buffer;
}

static void runFile(VM &vm, const char *source) {
}

int main(int argc, char *argv[]) {
  VM vm;

  if (argc == 2) {
    runFile(vm, argv[1]);
  } else {
    fprintf(stderr, "Usage: loxy [script]\n");
    exit(64);
  }
  exit(0);
}