#include <cstring>
#include "Value.h"

namespace loxy {

String *String::create(VM &vm, const char *chars, int length) {
  Hash hash = hashString(chars);
  void *mem = vm.newObject(sizeof(String));
  
  length = length == -1 ? strlen(chars) : length;
  std::unique_ptr<char> rawStr(reinterpret_cast<char*>(vm.newObject(length + 1)));
  memcpy(rawStr.get(), chars, length);
  (rawStr.get())[length] = '\0';

  String *strObj = ::new(mem) String(std::move(rawStr), length, hash);
  return strObj;
}

} // namespace loxy