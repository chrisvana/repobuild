// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_PY_LIBRARY_H__
#define _REPOBUILD_NODES_PY_LIBRARY_H__

#include <map>
#include <string>
#include <vector>
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
  virtual void LocalDependencyFiles(LanguageType lang,
                                    ResourceFileSet* files) const;
  virtual void LocalObjectFiles(LanguageType lang,
                                ResourceFileSet* files) const;
  virtual void ExternalDependencyFiles(
      LanguageType lang,
      std::map<std::string, std::string>* files) const;

  static void FinishMakeFile(const Input& input,
                             const std::vector<const Node*>& all_nodes,
                             Makefile* out);

  // For manual construction.
  void Set(const std::vector<Resource>& sources);

 protected:
  void Init();
  void LocalWriteMakeInternal(bool write_user_target,
                              Makefile* out) const;
  Resource PyFileFor(const Resource& r) const;

  Resource touchfile_;
  std::string py_base_dir_;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_PY_LIBRARY_H__
