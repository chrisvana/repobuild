// Copyright 2013
// Author: Christopher Van Arsdale

#include <stdlib.h>
#include <memory>
#include <string>
#include <map>
#include "common/base/init.h"
#include "common/base/flags.h"
#include "common/log/log.h"
#include "common/util/stl.h"
#include "common/strings/path.h"
#include "common/strings/strutil.h"
#include "common/util/stl.h"
#include "repobuild/distsource/git_tree.h"
extern "C" {
#include "repobuild/third_party/libgit2/include/git2.h"
}

using std::map;
using std::string;
using std::unique_ptr;

namespace repobuild {
namespace {

#define GIT_FREE(x, p, f)                                        \
  struct GitFree_##x {                                           \
    void operator()(p *ptr) const {                              \
      if (ptr) {                                                 \
        f(ptr);                                                  \
      }                                                          \
    }                                                            \
  };
GIT_FREE(Repo, git_repository, git_repository_free);
GIT_FREE(Index, git_index, git_index_free);

typedef unique_ptr<git_repository, GitFree_Repo> ScopedGitRepo;
typedef unique_ptr<git_index, GitFree_Index> ScopedGitIndex;

#undef GIT_FREE

git_repository* OpenRepo(const string& path) {
  git_repository* repo_ptr = NULL;
  int error = git_repository_open(&repo_ptr, path.c_str());
  ScopedGitRepo repo(repo_ptr);
  if (error != 0) {
    const char* error = (giterr_last() && giterr_last()->message ?
                         giterr_last()->message : "???");
    VLOG(1) << "Git Open error: " << error;
    return NULL;
  }
  return repo.release();
}

git_index* OpenIndex(git_repository* repo) {
  git_index* index_ptr = NULL;
  int error = git_repository_index(&index_ptr, repo);
  ScopedGitIndex index(index_ptr);
  if (error != 0) {
    const char* error = (giterr_last() && giterr_last()->message ?
                         giterr_last()->message : "???");
    VLOG(1) << "Git Index error: " << error;
    return NULL;
  }
  git_index_read(index.get());
  return index.release();
}

}  // anonymous namespace

struct GitTree::GitData {
  ScopedGitRepo repo;
  ScopedGitIndex index;
};

GitTree::GitTree(const string& root_path)
    : root_dir_(root_path) {
  Reset();
}

void GitTree::Reset() {
  data_.reset(new GitData);

  // Initialize git.
  data_->repo.reset(OpenRepo(root_dir_));
  if (data_->repo.get()) {
    data_->index.reset(OpenIndex(data_->repo.get()));
  }

  // Find all of our submodules.
  if (data_->index.get() != NULL) {
    int count = git_index_entrycount(data_->index.get());
    for (int i = 0; i < count; ++i) {
      const git_index_entry *e = git_index_get_byindex(data_->index.get(), i);
      if (e->mode == 0xE000 /* special submodule identifier */) {
        children_[e->path] = new GitTree(strings::JoinPath(root_dir_, e->path));
      }
    }
  }
}

GitTree::~GitTree() {
  DeleteValues(&children_);
}

bool GitTree::Initialized() const {
  return data_->index.get() != NULL;
}

void GitTree::ExpandChild(const string& path) {
  bool found_anything = false;
  for (auto it : children_) {
    const string& submodule = it.first;
    GitTree* tree = it.second;
    // TODO(cvanarsdale): Globs would be nice here. However, it's not exactly
    // trivial to glob against a prefix. You could probably pop path components
    // off of 'path' and use a glob library to match the substring against
    // "submodule".
    if (strings::HasPrefix(path, submodule)) {
      found_anything = true;
      if (!tree->Initialized()) {
        InitializeSubmodule(submodule, tree);
      }
      tree->ExpandChild(path.substr(submodule.size() + 1));
    }
  }
  VLOG_IF(1, !found_anything) << "Path not found in submodules: " << path;
}

void GitTree::InitializeSubmodule(const string& submodule, GitTree* sub_tree) {
  LOG(INFO) << "Initializing submodule: " << submodule;
  // NB: Why use 'git' here instead of libgit2? This is to avoid requiring
  // a bunch of libraries (ssl, ssh, zlib) needed to make git work correctly.
  int retval = system(strings::Join(
      "(cd ", root_dir_, "; ",
      "git submodule update --init ", submodule, ")").c_str());
  if (retval != 0) {
    LOG(ERROR) << "Could not expand submodule: "
               << strings::JoinPath(root_dir_, submodule)
               << ". Possible git error.";
  }
  sub_tree->Reset();
}

}  // namespace repobuild
