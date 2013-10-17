// Copyright 2013
// Author: Christopher Van Arsdale

#include <set>
#include <string>
#include <vector>
#include "repobuild/env/input.h"
#include "repobuild/env/resource.h"
#include "repobuild/nodes/execute_test.h"
#include "repobuild/nodes/gen_sh.h"

using std::set;
using std::string;
using std::vector;

namespace repobuild {
  
ExecuteTestNode::ExecuteTestNode(const TargetInfo& target,
                                 const Input& input,
                                 DistSource* source)
    : Node(target.GetParallelTarget(target.local_path() + ".test"),
           input,
           source),
      orig_target_(target) {
}

ExecuteTestNode::~ExecuteTestNode() {
}

void ExecuteTestNode::LocalWriteMake(Makefile* out) const {
  WriteBaseUserTarget(out);
}

void ExecuteTestNode::LocalTests(LanguageType lang,
                                 set<string>* targets) const {
  targets->insert(target().make_path());
}

void ExecuteTestNode::AddShNodes(BuildFile* file, Node* binary_node) {
  ResourceFileSet binaries;
  binary_node->TopTestBinaries(Node::NO_LANG, &binaries);

  for (const Resource& r : binaries) {
    GenShNode* node = NewSubNode<GenShNode>(file);
    node->SetMakeName("Testing");
    node->SetMakeTarget(r.path());

    vector<Resource> inputs(1, r), outputs;
    node->Set("$ROOT_DIR/" + r.path(),  // binary target.
              "",  // clean command
              inputs,
              outputs);
    node->AddDependencyTarget(binary_node->target());
  }
}

}  // namespace repobuild
