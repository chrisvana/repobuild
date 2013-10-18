// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_DISTSOURCE_DIST_SOURCE_IMPL_H__
#define _REPOBUILD_DISTSOURCE_DIST_SOURCE_IMPL_H__

#include <memory>
#include <string>
#include <vector>
#include "common/base/macros.h"
#include "repobuild/distsource/dist_source.h"

namespace repobuild {
class GitTree;
class Input;

class DistSourceImpl : public DistSource {
 public:
  explicit DistSourceImpl(const std::string& root_dir);
  virtual ~DistSourceImpl();

  virtual void InitializeForFile(const std::string& glob,
                                 std::vector<std::string>* files);
  virtual void WriteMakeFile(Makefile* out);
  virtual void WriteMakeClean(Makefile::Rule* out);
  virtual void WriteMakeHead(const Input& input, Makefile* out);

 private:
  DISALLOW_COPY_AND_ASSIGN(DistSourceImpl);

  std::unique_ptr<GitTree> git_tree_;
};

}  //  namespace repobuild

#endif  // _REPOBUILD_DISTSOURCE_DIST_SOURCE_IMPL_H__
