// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_PY_LIBRARY_H__
#define _REPOBUILD_NODES_PY_LIBRARY_H__

#include "repobuild/nodes/node.h"

namespace repobuild {

class PyLibraryNode : public SimpleLibraryNode {
 public:
  PyLibraryNode(const TargetInfo& t,
                const Input& i)
      : SimpleLibraryNode(t, i) {
  }
  virtual ~PyLibraryNode() {}
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void WriteMakefile(const std::vector<const Node*>& all_deps,
                             Makefile* out) const {
    WriteMakefileInternal(all_deps, true, out);
  }
  virtual void DependencyFiles(std::vector<Resource>* files) const;

 protected:
  void WriteMakefileInternal(const std::vector<const Node*>& all_deps,
                             bool write_user_target,
                             Makefile* out) const;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_PY_LIBRARY_H__
