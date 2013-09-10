// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_CONFIGNODE_H__
#define _REPOBUILD_NODES_CONFIGNODE_H__

#include <string>
#include "repobuild/nodes/node.h"

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
  virtual void WriteMakeClean(const std::vector<const Node*>& all_deps,
                              Makefile* out) const;
  virtual void WriteMakefile(const std::vector<const Node*>& all_deps,
                             Makefile* out) const;
  virtual void DependencyFiles(std::set<Resource>* files) const;

 protected:
  void AddSymlink(const std::string& dir,
                  const std::string& source,
                  Makefile* out) const;
  std::string DummyFile(const std::string& dir) const;
  std::string SourceDir(const std::string& middle) const;
  std::string CurrentDir(const std::string& middle) const;

  std::string component_src_, component_root_;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_CONFIGNODE_H__
