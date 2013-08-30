// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_CC_BINARY_H__
#define _REPOBUILD_NODES_CC_BINARY_H__

#include <string>
#include <vector>
#include "repobuild/nodes/node.h"
#include "repobuild/nodes/cc_library.h"

namespace repobuild {

class CCBinaryNode : public CCLibraryNode {
 public:
  CCBinaryNode(const TargetInfo& t,
               const Input& i)
      : CCLibraryNode(t, i) {
  }
  virtual ~CCBinaryNode() {}
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void WriteMakeClean(const std::vector<const Node*>& all_deps,
                              Makefile* out) const;
  virtual void WriteMakefile(const std::vector<const Node*>& all_deps,
                             Makefile* out) const;
  virtual void FinalOutputs(std::vector<Resource>* outputs) const;

 protected:
  // Helper.
  Resource OutBinary() const;

  void WriteLink(const std::vector<const Node*>& all_deps,
                 const Resource& file,
                 Makefile* out) const;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_CC_BINARY_H__
