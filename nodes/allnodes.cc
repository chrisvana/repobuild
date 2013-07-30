// Copyright 2013
// Author: Christopher Van Arsdale

#include <vector>
#include "nodes/allnodes.h"
#include "nodes/node.h"
#include "nodes/cc_library.h"
#include "nodes/cc_binary.h"
#include "nodes/confignode.h"
#include "nodes/gen_sh.h"

namespace repobuild {
namespace {
template <typename T>
class NodeBuilderImpl : public NodeBuilder {
 public:
  NodeBuilderImpl(const std::string& name) : name_(name) {}
  virtual std::string Name() const { return name_; }
  virtual Node* NewNode(const TargetInfo& target,
                        const Input& input) {
    return new T(target, input);
  }
 private:
  std::string name_;
};
}

// static
void NodeBuilder::GetAll(std::vector<NodeBuilder*>* nodes) {
  nodes->push_back(new NodeBuilderImpl<CCLibraryNode>("cc_library"));
  nodes->push_back(new NodeBuilderImpl<CCBinaryNode>("cc_binary"));
  nodes->push_back(new NodeBuilderImpl<ConfigNode>("config"));
  nodes->push_back(new NodeBuilderImpl<GenShNode>("gen_sh"));
}

}  // namespace repobuild
