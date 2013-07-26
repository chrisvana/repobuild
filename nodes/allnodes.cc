// Copyright 2013
// Author: Christopher Van Arsdale

#include <vector>
#include "nodes/allnodes.h"
#include "nodes/node.h"
#include "nodes/cc_library.h"
#include "nodes/cc_binary.h"

namespace repobuild {
namespace {
template <typename T>
class NodeBuilderImpl : public NodeBuilder {
 public:
  NodeBuilderImpl(const std::string& name) : name_(name) {}
  virtual std::string Name() const { return name_; }
  virtual Node* NewNode(const TargetInfo& target) {
    return new T(target);
  }
 private:
  std::string name_;
};
}

void GetAll(std::vector<NodeBuilder*>* nodes) {
  nodes->push_back(new NodeBuilderImpl<CCLibraryNode>("cc_library"));
  nodes->push_back(new NodeBuilderImpl<CCBinaryNode>("cc_binary"));
}

}  // namespace repobuild
