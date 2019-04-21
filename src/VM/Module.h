#ifndef loxy_module_h
#define loxy_module_h

#include <vector>
#include <map>
#include <memory>

namespace loxy {

class String;
class Chunk;
class Value;
class VM;

typedef std::map<String*, Value> SymbolTable;

class Module {
public:

  Module(String *name, String *path, std::unique_ptr<char> src)
  : name_(name),
    path_(path),
    src_(std::move(src)),
    bytecode_(nullptr) {}

  // adds a top-level variable. Global variables can be redefined.
  void addVariable(String *name, Value initializer);

  // gets a top-level variable. returns [true] indicating success.
  // value will be written to [result] only when this method returns [true].
  bool getVariable(String *name, Value *result);

  // module's name.
  String *getName() const { return name_; }
  void setName(String *name) { name_ = name; }

  String *getPath() const { return path_; }
  void setPath(String *path) { path_ = path; }

  void addImports(Module *module) { imports_.push_back(module); }

  // compiles from [src_].
  bool compile(VM &vm);

  // source code.
  const char *getSrc() const { return src_.get(); }
  void setSrc(std::unique_ptr<char> src) { src_ = std::move(src); }

  // the compiled bytecode of this module.
  const Chunk *getBody() const { return bytecode_.get(); }
  void setBody(std::unique_ptr<Chunk> body) { bytecode_ = std::move(body); }

private:

  // the name of module.
  String *name_;

  // the path where the module was loaded from.
  String *path_;

  // source code of the module.
  std::unique_ptr<char> src_;

  // the compiled bytecode.
  std::unique_ptr<Chunk> bytecode_;

  // top-level variables.
  SymbolTable variables;

  // imported modules
  std::vector<Module*> imports_;
};

} // namespace loxy


#endif