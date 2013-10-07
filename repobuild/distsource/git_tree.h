// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_DISTSOURCE_GIT_TREE_H__
#define _REPOBUILD_DISTSOURCE_GIT_TREE_H__

#include <memory>
#include <string>
#include <map>
#include "common/base/macros.h"

namespace repobuild {

class GitTree {
 public:
  explicit GitTree(const std::string& root_path);
  ~GitTree();

  bool Initialized() const;
  void ExpandChild(const std::string& path);

 private:
  void InitializeSubmodule(const std::string& submodule, GitTree* sub_tree);
  void Reset();
  DISALLOW_COPY_AND_ASSIGN(GitTree);

  struct GitData;
  std::string root_dir_;
  std::unique_ptr<GitData> data_;
  std::map<std::string, GitTree*> children_;
};

}  // namespace repobuild

#endif  // _REPOBUILD_DISTSOURCE_GIT_TREE_H__
