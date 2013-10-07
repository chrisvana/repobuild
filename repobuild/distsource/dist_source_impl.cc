// Copyright 2013
// Author: Christopher Van Arsdale

#include <string>
#include "repobuild/distsource/dist_source_impl.h"
#include "repobuild/distsource/git_tree.h"

using std::string;

namespace repobuild {

DistSourceImpl::DistSourceImpl(const std::string& root_dir)
    : git_tree_(new GitTree(root_dir)) {
}

DistSourceImpl::~DistSourceImpl() {
}

void DistSourceImpl::InitializeForFile(const std::string& file) {
  // NOTE(cvanarsdale): Eventually we may want to initialize FUSE file systems
  // here, svn checkout, hg, etc.
  git_tree_->ExpandChild(file);
}

}  //  namespace repobuild
