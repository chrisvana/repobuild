// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_MAKE_H__
#define _REPOBUILD_NODES_MAKE_H__

#include <string>
#include "repobuild/nodes/node.h"

namespace repobuild {

class MakeNode : public Node {
 public:
  MakeNode(const TargetInfo& t,
           const Input& i,
           DistSource* s)
      : Node(t, i, s) {
  }
  virtual ~MakeNode() {}
  virtual void Parse(BuildFile* file, const BuildFileNode& input) {
    ParseWithOptions(file, input, "", "$GEN_DIR", "");
  }
  void ParseWithOptions(BuildFile* file,
                        const BuildFileNode& input,
                        const std::string& preinstall,
                        const std::string& dest_dir,
                        const std::string& postinstall);
  virtual void LocalWriteMake(Makefile* out) const {
    WriteBaseUserTarget(out);
  }
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_MAKE_H__
