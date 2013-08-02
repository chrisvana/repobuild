// Copyright 2013
// Author: Christopher Van Arsdale

#include <string>
#include <set>
#include <iterator>
#include <vector>
#include "common/log/log.h"
#include "common/file/fileutil.h"
#include "common/strings/path.h"
#include "repobuild/env/input.h"
#include "nodes/go_library.h"
#include "repobuild/reader/buildfile.h"

using std::vector;
using std::string;

namespace repobuild {

void GoLibraryNode::Parse(BuildFile* file, const BuildFileNode& input) {
  Node::Parse(file, input);

  // go_sources
  ParseRepeatedFiles(input, "go_sources", &sources_);
}

void GoLibraryNode::Set(const vector<string>& sources) {
  sources_ = sources;
}

void GoLibraryNode::DependencyFiles(vector<string>* files) const {
  Node::DependencyFiles(files);
  for (int i = 0; i < sources_.size(); ++i) {
    files->push_back(sources_[i]);
  }
}

}  // namespace repobuild
