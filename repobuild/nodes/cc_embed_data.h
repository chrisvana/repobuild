// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_CC_EMBED_DATA_H__
#define _REPOBUILD_NODES_CC_EMBED_DATA_H__

#include <string>
#include <vector>
#include "repobuild/nodes/node.h"

namespace repobuild {

class CCEmbedDataNode : public Node {
 public:
  CCEmbedDataNode(const TargetInfo& target,
                  const Input& input,
                  DistSource* source);
  virtual ~CCEmbedDataNode() {}
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void LocalWriteMake(Makefile* out) const {
    WriteBaseUserTarget(out);
  }

  static void WriteMakeHead(const Input& input, Makefile* out);
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_CC_EMBED_DATA_H__
