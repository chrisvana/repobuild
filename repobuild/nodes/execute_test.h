// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_EXECUTE_TEST_H__
#define _REPOBUILD_NODES_EXECUTE_TEST_H__

#include <set>
#include <string>
#include <vector>
#include "repobuild/env/resource.h"
#include "repobuild/nodes/node.h"
#include "repobuild/reader/buildfile.h"

namespace repobuild {

class ExecuteTestNode : public Node {
 public:
  ExecuteTestNode(const TargetInfo& target, const Input& input);
  virtual ~ExecuteTestNode();

  virtual bool IncludeInAll() const { return false; }
  virtual bool IncludeInTests() const { return true; }
  virtual void LocalWriteMake(Makefile* out) const;
  virtual void LocalTests(LanguageType lang,
                          std::set<std::string>* targets) const;

 protected:
  void AddShNodes(BuildFile* file, Node* binary_node);

  TargetInfo orig_target_;
};

template <class T>
class ExecuteTestNodeImpl : public ExecuteTestNode {
 public:
  ExecuteTestNodeImpl(const TargetInfo& target, const Input& input)
      : ExecuteTestNode(target, input) {
  }
  virtual ~ExecuteTestNodeImpl() {}

  virtual void Parse(BuildFile* file, const BuildFileNode& input) {
    // binary node
    Node* subnode = new T(orig_target_, Node::input());
    subnode->Parse(file, input);
    AddSubNode(subnode);

    // test node.
    AddShNodes(file, subnode);
  }
};

#endif  // _REPOBUILD_NODES_EXECUTE_TEST_H__

}  // namespace repobuild
