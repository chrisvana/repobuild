// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_DISTSOURCE_DIST_SOURCE_IMPL_H__
#define _REPOBUILD_DISTSOURCE_DIST_SOURCE_IMPL_H__

#include <memory>
#include <string>
#include "repobuild/distsource/dist_source.h"

namespace repobuild {
class GitTree;

class DistSourceImpl : public DistSource {
 public:
  explicit DistSourceImpl(const std::string& root_dir);
  virtual ~DistSourceImpl();

  virtual void InitializeForFile(const std::string& file);

 private:
  std::unique_ptr<GitTree> git_tree_;
};

}  //  namespace repobuild

#endif  // _REPOBUILD_DISTSOURCE_DIST_SOURCE_IMPL_H__
