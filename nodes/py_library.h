// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_PY_LIBRARY_H__
#define _REPOBUILD_NODES_PY_LIBRARY_H__

#include "repobuild/nodes/node.h"
#include "repobuild/env/resource.h"

namespace repobuild {

class PyLibraryNode : public SimpleLibraryNode {
 public:
  PyLibraryNode(const TargetInfo& t,
                const Input& i)
      : SimpleLibraryNode(t, i) {
  }
  virtual ~PyLibraryNode() {}
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void LocalWriteMake(Makefile* out) const {
    LocalWriteMakeInternal(true, out);
  }
  virtual void LocalDependencyFiles(ResourceFileSet* files) const;

  // For manual construction.
  void Set(const std::vector<Resource>& sources);

 protected:
  void Init();
  void LocalWriteMakeInternal(bool write_user_target,
                              Makefile* out) const;

  Resource touchfile_;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_PY_LIBRARY_H__
