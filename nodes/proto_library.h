// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_PROTO_LIBRARY_H__
#define _REPOBUILD_NODES_PROTO_LIBRARY_H__

#include <string>
#include "nodes/node.h"

namespace repobuild {

class ProtoLibraryNode : public Node {
 public:
  ProtoLibraryNode(const TargetInfo& t,
                   const Input& i)
      : Node(t, i) {
  }
  virtual ~ProtoLibraryNode() {}
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void WriteMakefile(const std::vector<const Node*>& all_deps,
                             std::string* out) const {}
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_PROTO_LIBRARY_H__
