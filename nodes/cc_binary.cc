// Copyright 2013
// Author: Christopher Van Arsdale

#include "common/log/log.h"
#include "nodes/cc_binary.h"
#include "reader/buildfile.h"

namespace repobuild {

void CCBinaryNode::Parse(const BuildFile& file, const BuildFileNode& input) {
  CCLibraryNode::Parse(file, input);
  // TODO
  LOG(FATAL) << "TODO";
}

}  // namespace repobuild
