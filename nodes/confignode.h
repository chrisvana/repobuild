// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_CONFIGNODE_H__
#define _REPOBUILD_NODES_CONFIGNODE_H__

#include <string>
#include "nodes/node.h"

namespace repobuild {

class ConfigNode : public Node {
 public:
  ConfigNode(const TargetInfo& t,
             const Input& i)
      : Node(t, i) {
  }
  virtual ~ConfigNode() {}
  virtual std::string Name() const { return "config"; }
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void WriteMakeClean(std::string* out) const;
  virtual void WriteMakefile(const std::vector<const Node*>& all_deps,
                             std::string* out) const;
  virtual void DependencyFiles(std::vector<std::string>* files) const;

 protected:
  std::string DummyFile() const;
  std::string SourceDir() const;

  std::string component_src_, component_root_;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_CONFIGNODE_H__
