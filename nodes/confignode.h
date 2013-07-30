// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_CONFIGNODE_H__
#define _REPOBUILD_NODES_CONFIGNODE_H__

#include <string>
#include "nodes/node.h"

namespace repobuild {

class ConfigNode : public Node {
 public:
  ConfigNode(const TargetInfo& t) : Node(t) {}
  virtual ~ConfigNode() {}
  virtual std::string Name() const { return "config"; }
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void WriteMakeClean(const Input& input, std::string* out) const;
  virtual void WriteMakefile(const Input& input,
                             const std::vector<const Node*>& all_deps,
                             std::string* out) const;
  virtual void DependencyFiles(const Input& input,
                               std::vector<std::string>* files) const;
 protected:
  std::string component_src_root_;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_CONFIGNODE_H__
