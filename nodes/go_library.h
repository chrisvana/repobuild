// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_GO_LIBRARY_H__
#define _REPOBUILD_NODES_GO_LIBRARY_H__

#include "repobuild/nodes/node.h"

namespace repobuild {

class GoLibraryNode : public SimpleLibraryNode {
 public:
  GoLibraryNode(const TargetInfo& t,
                const Input& i)
      : SimpleLibraryNode(t, i) {
  }
  virtual ~GoLibraryNode() {}
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_GO_LIBRARY_H__
