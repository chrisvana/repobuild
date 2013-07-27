// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_ALLNODES_H__
#define _REPOBUILD_NODES_ALLNODES_H__

#include <vector>
#include "nodes/node.h"

namespace repobuild {

class NodeBuilder {
 public:
  virtual ~NodeBuilder() {}
  virtual std::string Name() const = 0;
  virtual Node* NewNode(const TargetInfo& target) = 0;

  static void GetAll(std::vector<NodeBuilder*>* nodes);
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_ALLNODES_H__
