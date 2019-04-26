#ifndef loxy_module_h
#define loxy_module_h

#include "Data/HashMap.h"
#include "Data/SmallVector.h"
#include "Managed.h"

namespace loxy {

class String;
class Chunk;
class Value;
class VM;
class HashMap;

class Module : public Managed {
private:

  Module(VM &vm,
        String *name,
        String *path,
        String *src,
        HashMap *variables,
        SmallVector<Module*> *imports)
  : vm(vm),
    name_(name),
    path_(path),
    src_(src),
    bytecode_(nullptr),
    variables_(variables),
    imports_(imports) {}

public:
  static Module *create(VM &vm, String *name, String *path, String *src);
  static void destroy(VM &vm, Module **module);

  // adds a top-level variable. Global variables can be redefined.
  void addVariable(String *name, Value initializer);

  // gets a top-level variable. returns [true] indicating success.
  // value will be written to [result] only when this method returns [true].
  bool getVariable(String *name, Value *result);

  bool setVariable(String *name, Value value);

  // module's name.
  String *getName() const { return name_; }
  void setName(String *name) { name_ = name; }

  String *getPath() const { return path_; }
  void setPath(String *path) { path_ = path; }

  void addImports(Module *module) { imports_->push(module); }

  // compiles from [src_].
  bool compile();

  // source code.
  const char *getSrc() const { return src_->cString(); }
  void setSrc(String *src) { src_ = src; }

  // the compiled bytecode of this module.
  const Chunk *getBody() const { return bytecode_; }
  void setBody(Chunk *body) { bytecode_ = body; }

private:

  VM &vm;

  // the name of module.
  String *name_;

  // the path where the module was loaded from.
  String *path_;

  // source code of the module.
  String *src_;

  // the compiled bytecode.
  Chunk *bytecode_;

  // TODO:
  // change this to an array.
  // top-level variables.
  HashMap *variables_;

  // imported modules
  SmallVector<Module*> *imports_;
};

} // namespace loxy


#endif