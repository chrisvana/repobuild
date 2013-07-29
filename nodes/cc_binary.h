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
  CCBinaryNode(const TargetInfo& t) : CCLibraryNode(t) {}
  virtual ~CCBinaryNode() {}
  virtual std::string Name() const { return "cc_binary"; }
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void WriteMakefile(const Input& input,
                             const std::vector<const Node*>& all_deps,
                             std::string* out) const;

 protected:
  std::vector<std::string> cc_linker_args_;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_CC_BINARY_H__
