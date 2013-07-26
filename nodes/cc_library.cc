// Copyright 2013
// Author: Christopher Van Arsdale

#include "common/log/log.h"
#include "nodes/cc_library.h"
#include "reader/buildfile.h"

namespace repobuild {

void CCLibraryNode::Parse(const BuildFile& file, const BuildFileNode& input) {
  Node::Parse(file, input);
  // TODO
  LOG(FATAL) << "TODO";
}

}  // namespace repobuild
