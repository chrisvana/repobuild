// Copyright 2013
// Author: Christopher Van Arsdale

#include <string>
#include <vector>
#include "common/file/fileutil.h"
#include "common/log/log.h"
#include "repobuild/distsource/dist_source_impl.h"
#include "repobuild/distsource/git_tree.h"

using std::string;
using std::vector;

namespace repobuild {

DistSourceImpl::DistSourceImpl(const string& root_dir)
    : git_tree_(new GitTree(root_dir)) {
}

DistSourceImpl::~DistSourceImpl() {
}

void DistSourceImpl::InitializeForFile(const string& glob,
                                       vector<string>* files) {
  // NOTE(cvanarsdale): Eventually we may want to initialize FUSE file systems
  // here, svn checkout, hg, etc.
  git_tree_->ExpandChild(glob);
  vector<string> tmp;
  CHECK(file::Glob(glob, &tmp))
      << "Could not run glob(" << glob << "), bad filesystem permissions?";
  for (const string& file : tmp) {
    git_tree_->RecordFile(file);
    if (files != NULL) {
      files->push_back(file);
    }
  }
}

void DistSourceImpl::WriteMakeFile(Makefile* out) {
  git_tree_->WriteMakeFile(out);
}

void DistSourceImpl::WriteMakeClean(Makefile::Rule* out) {
  git_tree_->WriteMakeClean(out);
}

}  //  namespace repobuild
