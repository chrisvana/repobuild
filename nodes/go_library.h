// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_GO_LIBRARY_H__
#define _REPOBUILD_NODES_GO_LIBRARY_H__

#include <string>
#include "nodes/node.h"

namespace repobuild {

class GoLibraryNode : public Node {
 public:
  GoLibraryNode(const TargetInfo& t,
                const Input& i)
      : Node(t, i) {
  }
  virtual ~GoLibraryNode() {}
  virtual void WriteMakefile(const std::vector<const Node*>& all_deps,
                             std::string* out) const {}
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void DependencyFiles(std::vector<std::string>* files) const;

  // Alterative to Parse()
  void Set(const std::vector<std::string>& sources);

 protected:
  std::vector<std::string> sources_;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_GO_LIBRARY_H__
