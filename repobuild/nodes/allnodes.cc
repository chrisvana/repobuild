// Copyright 2013
// Author: Christopher Van Arsdale

#include <vector>
#include "common/log/log.h"
#include "common/util/stl.h"
#include "repobuild/nodes/allnodes.h"
#include "repobuild/nodes/autoconf.h"
#include "repobuild/nodes/cmake.h"
#include "repobuild/nodes/node.h"
#include "repobuild/nodes/cc_library.h"
#include "repobuild/nodes/cc_binary.h"
#include "repobuild/nodes/cc_embed_data.h"
#include "repobuild/nodes/cc_shared_library.h"
#include "repobuild/nodes/confignode.h"
#include "repobuild/nodes/execute_test.h"
#include "repobuild/nodes/go_library.h"
#include "repobuild/nodes/go_binary.h"
#include "repobuild/nodes/go_test.h"
#include "repobuild/nodes/gen_sh.h"
#include "repobuild/nodes/java_library.h"
#include "repobuild/nodes/java_jar.h"
#include "repobuild/nodes/java_binary.h"
#include "repobuild/nodes/make.h"
#include "repobuild/nodes/plugin.h"
#include "repobuild/nodes/proto_library.h"
#include "repobuild/nodes/py_binary.h"
#include "repobuild/nodes/py_egg.h"
#include "repobuild/nodes/py_library.h"

using std::string;
using std::vector;

namespace repobuild {
namespace {
template <typename T>
class NodeBuilderImpl : public NodeBuilder {
 public:
  NodeBuilderImpl(const std::string& name) : name_(name) {}
  virtual std::string Name() const { return name_; }
  virtual Node* NewNode(const TargetInfo& target,
                        const Input& input,
                        DistSource* source) {
    return new T(target, input, source);
  }
  virtual void WriteMakeHead(const Input& input, Makefile* out) {}
  virtual void FinishMakeFile(const Input& input,
                              const vector<const Node*>& all_nodes,
                              DistSource* source,
                              Makefile* out) {}

 private:
  std::string name_;
};

template <typename T>
class NodeBuilderImplHead : public NodeBuilderImpl<T> {
 public:
  NodeBuilderImplHead(const std::string& name) : NodeBuilderImpl<T>(name) {}
  virtual void WriteMakeHead(const Input& input, Makefile* out) {
    T::WriteMakeHead(input, out);
  }
};

template <typename T>
class NodeBuilderImplFinish : public NodeBuilderImpl<T> {
 public:
  NodeBuilderImplFinish(const std::string& name) : NodeBuilderImpl<T>(name) {}
  virtual void FinishMakeFile(const Input& input,
                              const vector<const Node*>& all_nodes,
                              DistSource* source,
                              Makefile* out) {
    T::FinishMakeFile(input, all_nodes, source, out);
  }
};
}

// static
void NodeBuilder::GetAll(std::vector<NodeBuilder*>* nodes) {
  nodes->push_back(new NodeBuilderImplHead<GenShNode>("gen_sh"));
  nodes->push_back(new NodeBuilderImplHead<CCLibraryNode>("cc_library"));
  nodes->push_back(new NodeBuilderImplHead<CCSharedLibraryNode>(
      "cc_shared_library"));
  nodes->push_back(new NodeBuilderImplHead<PyBinaryNode>("py_egg"));
  nodes->push_back(new NodeBuilderImplHead<CCEmbedDataNode>("cc_embed_data"));

  nodes->push_back(new NodeBuilderImpl<AutoconfNode>("autoconf"));
  nodes->push_back(new NodeBuilderImpl<CCBinaryNode>("cc_binary"));
  nodes->push_back(new NodeBuilderImpl<CmakeNode>("cmake"));
  nodes->push_back(new NodeBuilderImpl<ConfigNode>("config"));
  nodes->push_back(new NodeBuilderImpl<GoLibraryNode>("go_library"));
  nodes->push_back(new NodeBuilderImpl<GoBinaryNode>("go_binary"));
  nodes->push_back(new NodeBuilderImpl<JavaLibraryNode>("java_library"));
  nodes->push_back(new NodeBuilderImpl<JavaJarNode>("java_jar"));
  nodes->push_back(new NodeBuilderImpl<JavaBinaryNode>("java_binary"));
  nodes->push_back(new NodeBuilderImpl<MakeNode>("make"));
  nodes->push_back(new NodeBuilderImpl<PluginNode>("plugin"));
  nodes->push_back(new NodeBuilderImpl<ProtoLibraryNode>("proto_library"));
  nodes->push_back(new NodeBuilderImpl<PyBinaryNode>("py_binary"));

  nodes->push_back(new NodeBuilderImplFinish<PyLibraryNode>("py_library"));

  // Test nodes
  nodes->push_back(new NodeBuilderImpl<ExecuteTestNodeImpl<CCBinaryNode> >(
      "cc_test"));
  nodes->push_back(new NodeBuilderImpl<ExecuteTestNodeImpl<PyBinaryNode> >(
      "py_test"));
  nodes->push_back(new NodeBuilderImpl<ExecuteTestNodeImpl<JavaBinaryNode> >(
      "java_test"));
  nodes->push_back(new NodeBuilderImpl<GoTestNode>("go_test"));
}

NodeBuilderSet::NodeBuilderSet() {
  vector<NodeBuilder*> nodes;
  NodeBuilder::GetAll(&nodes);
  Init(nodes);
}

NodeBuilderSet::NodeBuilderSet(const vector<NodeBuilder*>& nodes) {
  Init(nodes);
}

NodeBuilderSet::~NodeBuilderSet() {
  DeleteElements(&all_nodes_);
}

void NodeBuilderSet::Init(const vector<NodeBuilder*>& nodes) {
  all_nodes_ = nodes;  // in order copy.
  for (NodeBuilder* n : nodes) {
    NodeBuilder** it = &nodes_[n->Name()];
    CHECK(*it == NULL) << "Duplicate node builder: " << n->Name();
    *it = n;
  }
}

Node* NodeBuilderSet::NewNode(const string& name,
                              const TargetInfo& target,
                              const Input& input,
                              DistSource* source) const {
  auto it = nodes_.find(name);
  if (it == nodes_.end()) {
    return NULL;
  }
  return it->second->NewNode(target, input, source);
}

void NodeBuilderSet::WriteMakeHead(const Input& input, Makefile* makefile) {
  for (NodeBuilder* builder : all_nodes_) {
    builder->WriteMakeHead(input, makefile);
  }
}

void NodeBuilderSet::FinishMakeFile(const Input& input,
                                    const vector<const Node*>& nodes,
                                    DistSource* source,
                                    Makefile* makefile) {
  for (NodeBuilder* builder : all_nodes_) {
    builder->FinishMakeFile(input, nodes, source, makefile);
  }
}

}  // namespace repobuild
