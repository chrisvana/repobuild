// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_TOP_SYMLINK_H__
#define _REPOBUILD_NODES_TOP_SYMLINK_H__

#include <vector>
#include "repobuild/env/resource.h"
#include "repobuild/nodes/node.h"

namespace repobuild {

class TopSymlinkNode : public Node {
 public:
  TopSymlinkNode(const TargetInfo& target,
                 const Input& input,
                 DistSource* source,
                 const ResourceFileSet& input_binaries);
  virtual ~TopSymlinkNode();
  virtual void LocalWriteMakeClean(Makefile::Rule* out) const;
  virtual void LocalWriteMake(Makefile* out) const;
  virtual void LocalFinalOutputs(LanguageType lang,
                                 ResourceFileSet* outputs) const;

 protected:
  ResourceFileSet InputBinaries() const { return input_binaries_; }
  ResourceFileSet OutBinaries() const;
  
 private:
  ResourceFileSet input_binaries_;
};

}  // namespace repobuild

#endif  // _REPOBUILD_NODES_TOP_SYMLINK_H__
