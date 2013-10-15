// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_PLUGIN_H__
#define _REPOBUILD_NODES_PLUGIN_H__

#include <string>
#include "repobuild/nodes/node.h"

namespace repobuild {

class PluginNode : public Node {
 public:
  PluginNode(const TargetInfo& target,
             const Input& input,
             DistSource* source)
      : Node(target, input, source) {
  }
  virtual ~PluginNode() {}
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void LocalWriteMake(Makefile* out) const;
  virtual bool ExpandBuildFileNode(BuildFile* file, BuildFileNode* node);

 protected:
  std::string command_;
};

}  // namespace repobuild

#endif  // _REPOBUILD_NODES_PLUGIN_H__
