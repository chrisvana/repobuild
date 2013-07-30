// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_CC_BINARY_H__
#define _REPOBUILD_NODES_CC_BINARY_H__

#include <string>
#include "nodes/node.h"
#include "nodes/cc_library.h"

namespace repobuild {

class CCBinaryNode : public CCLibraryNode {
 public:
  CCBinaryNode(const TargetInfo& t,
               const Input& i)
      : CCLibraryNode(t, i) {
  }
  virtual ~CCBinaryNode() {}
  virtual std::string Name() const { return "cc_binary"; }
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void WriteMakeClean(std::string* out) const;
  virtual void WriteMakefile(const std::vector<const Node*>& all_deps,
                             std::string* out) const;
  virtual void FinalOutputs(std::vector<std::string>* outputs) const;

 protected:
  // Helper.
  std::string OutBinary() const;

  std::vector<std::string> cc_linker_args_;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_CC_BINARY_H__
