// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_DISTSOURCE_GIT_TREE_H__
#define _REPOBUILD_DISTSOURCE_GIT_TREE_H__

#include <memory>
#include <map>
#include <set>
#include <string>
#include "common/base/macros.h"
#include "repobuild/nodes/makefile.h"

namespace repobuild {

class GitTree {
 public:
  explicit GitTree(const std::string& root_path);
  ~GitTree();

  bool Initialized() const;
  void ExpandChild(const std::string& path);
  void RecordFile(const std::string& path);
  void WriteMakeFile(Makefile* out) const;
  void WriteMakeClean(Makefile::Rule* out) const;

 private:
  void InitializeSubmodule(const std::string& submodule, GitTree* sub_tree);
  void Reset();
  void WriteMakeFile(Makefile* out,
                     const std::string& full_dir,
                     const std::string& parent) const;
  DISALLOW_COPY_AND_ASSIGN(GitTree);

  struct GitData;
  std::string root_dir_;
  std::unique_ptr<GitData> data_;
  std::map<std::string, GitTree*> children_;
  std::set<std::string> used_submodules_;
  std::set<std::string> seen_files_;
};

}  // namespace repobuild

#endif  // _REPOBUILD_DISTSOURCE_GIT_TREE_H__
