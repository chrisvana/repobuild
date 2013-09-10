// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_GO_BINARY_H__
#define _REPOBUILD_NODES_GO_BINARY_H__

#include <string>
#include <vector>
#include "repobuild/nodes/node.h"
#include "repobuild/nodes/go_library.h"

namespace repobuild {

class GoBinaryNode : public GoLibraryNode {
 public:
  GoBinaryNode(const TargetInfo& t,
               const Input& i)
      : GoLibraryNode(t, i) {
  }
  virtual ~GoBinaryNode() {}
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void WriteMakeClean(Makefile* out) const;
  virtual void WriteMakefile(Makefile* out) const;
  virtual void FinalOutputs(std::set<Resource>* outputs) const;

 protected:
  // Helper.
  Resource OutBinary() const;

  std::vector<std::string> go_build_args_;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_GO_BINARY_H__
