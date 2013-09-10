// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_PY_BINARY_H__
#define _REPOBUILD_NODES_PY_BINARY_H__

#include <string>
#include <vector>
#include "repobuild/nodes/node.h"
#include "repobuild/nodes/py_library.h"

namespace repobuild {

class PyBinaryNode : public PyLibraryNode {
 public:
  PyBinaryNode(const TargetInfo& t,
               const Input& i)
      : PyLibraryNode(t, i) {
  }
  virtual ~PyBinaryNode() {}
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void WriteMakeClean(const std::vector<const Node*>& all_deps,
                              Makefile* out) const;
  virtual void WriteMakefile(const std::vector<const Node*>& all_deps,
                             Makefile* out) const;
  virtual void FinalOutputs(std::set<Resource>* outputs) const;

  static void WriteMakeHead(const Input& input, Makefile* out);

 protected:
  // Helper.
  Resource OutBinary() const;
  Resource OutEgg() const;

  std::vector<std::string> py_build_args_;
  std::string py_version_, py_default_module_;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_PY_BINARY_H__
