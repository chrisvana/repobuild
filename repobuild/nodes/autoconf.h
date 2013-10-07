// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_AUTOCONF_H__
#define _REPOBUILD_NODES_AUTOCONF_H__

#include <string>
#include "repobuild/nodes/node.h"

namespace repobuild {

class AutoconfNode : public Node {
 public:
  AutoconfNode(const TargetInfo& t,
               const Input& i,
               DistSource* s)
      : Node(t, i, s) {
  }
  virtual ~AutoconfNode() {}
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void LocalWriteMake(Makefile* out) const {
    WriteBaseUserTarget(out);
  }
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_AUTOCONF_H__
