// Copyright 2013
// Author: Christopher Van Arsdale

#include "repobuild/env/input.h"
#include "repobuild/env/resource.h"
#include "repobuild/nodes/top_symlink.h"

namespace repobuild {

TopSymlinkNode::TopSymlinkNode(const TargetInfo& target,
                               const Input& input,
                               DistSource* source,
                               const ResourceFileSet& input_binaries)
    : Node(target, input, source),
      input_binaries_(input_binaries) {
}

TopSymlinkNode::~TopSymlinkNode() {
}

void TopSymlinkNode::LocalWriteMakeClean(Makefile::Rule* out) const {
  for (const Resource r : OutBinaries()) {
    out->MaybeRemoveSymlink(r.path());
  }
}

void TopSymlinkNode::LocalWriteMake(Makefile* out) const {
  for (const Resource& r : InputBinaries()) {
    // NOTE(mike) This extra local symlink causes collisions with
    // whatever happens to be in the directory when repobuild is
    // called. I also don't understand the need to have two output
    // symlinks, so I'm removing this and relying on a single output
    // in the bin directory.
    //Resource local = Resource::FromLocalPath(input().root_dir(), r.basename());
    //out->WriteRootSymlink(local.path(), r.path());
    Resource bin = Resource::FromLocalPath(input().binary_dir(), r.basename());
    out->WriteRootSymlink(bin.path(), r.path());
  }
  WriteBaseUserTarget(out);
}

void TopSymlinkNode::LocalFinalOutputs(LanguageType lang,
                                       ResourceFileSet* outputs) const {
  for (const Resource& r : OutBinaries()) {
    outputs->Add(r);
  }
}

ResourceFileSet TopSymlinkNode::OutBinaries() const {
  ResourceFileSet inputs = InputBinaries();
  ResourceFileSet output;
  for (const Resource& r : inputs) {
    // NOTE(mike) Neuter extra symlink.
    // output.Add(Resource::FromLocalPath(input().root_dir(), r.basename()));
    output.Add(Resource::FromLocalPath(input().binary_dir(), r.basename()));
  }
  return output;
}

}  // namespace repobuild
