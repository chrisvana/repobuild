// Copyright 2013
// Author: Christopher Van Arsdale

#include <string>
#include <vector>
#include "nodes/go_library.h"
#include "repobuild/reader/buildfile.h"

using std::vector;
using std::string;

namespace repobuild {

void GoLibraryNode::Parse(BuildFile* file, const BuildFileNode& input) {
  SimpleLibraryNode::Parse(file, input);

  // go_sources
  ParseRepeatedFiles(input, "go_sources", &sources_);
}

}  // namespace repobuild
