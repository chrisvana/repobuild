// Copyright 2013
// Author: Christopher Van Arsdale

#include <vector>
#include "repobuild/nodes/allnodes.h"
#include "repobuild/nodes/autoconf.h"
#include "repobuild/nodes/cmake.h"
#include "repobuild/nodes/node.h"
#include "repobuild/nodes/cc_library.h"
#include "repobuild/nodes/cc_binary.h"
#include "repobuild/nodes/confignode.h"
#include "repobuild/nodes/go_library.h"
#include "repobuild/nodes/go_binary.h"
#include "repobuild/nodes/gen_sh.h"
#include "repobuild/nodes/java_library.h"
#include "repobuild/nodes/java_binary.h"
#include "repobuild/nodes/make.h"
#include "repobuild/nodes/proto_library.h"
#include "repobuild/nodes/py_library.h"
#include "repobuild/nodes/py_binary.h"

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
  nodes->push_back(new NodeBuilderImpl<AutoconfNode>("autoconf"));
  nodes->push_back(new NodeBuilderImpl<CCLibraryNode>("cc_library"));
  nodes->push_back(new NodeBuilderImpl<CCBinaryNode>("cc_binary"));
  nodes->push_back(new NodeBuilderImpl<CmakeNode>("cmake"));
  nodes->push_back(new NodeBuilderImpl<ConfigNode>("config"));
  nodes->push_back(new NodeBuilderImpl<GoLibraryNode>("go_library"));
  nodes->push_back(new NodeBuilderImpl<GoBinaryNode>("go_binary"));
  nodes->push_back(new NodeBuilderImpl<GenShNode>("gen_sh"));
  nodes->push_back(new NodeBuilderImpl<JavaLibraryNode>("java_library"));
  nodes->push_back(new NodeBuilderImpl<JavaBinaryNode>("java_binary"));
  nodes->push_back(new NodeBuilderImpl<MakeNode>("make"));
  nodes->push_back(new NodeBuilderImpl<ProtoLibraryNode>("proto_library"));
  nodes->push_back(new NodeBuilderImpl<PyLibraryNode>("py_library"));
  nodes->push_back(new NodeBuilderImpl<PyBinaryNode>("py_binary"));
}

}  // namespace repobuild
