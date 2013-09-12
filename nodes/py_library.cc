// Copyright 2013
// Author: Christopher Van Arsdale

#include <set>
#include <string>
#include <vector>
#include "repobuild/nodes/py_library.h"
#include "repobuild/reader/buildfile.h"

using std::vector;
using std::string;
using std::set;

namespace repobuild {

void PyLibraryNode::Parse(BuildFile* file, const BuildFileNode& input) {
  SimpleLibraryNode::Parse(file, input);

  // py_sources
  current_reader()->ParseRepeatedFiles("py_sources", &sources_);

  Init();
}

void PyLibraryNode::LocalWriteMakeInternal(bool write_user_target,
                                           Makefile* out) const {
  SimpleLibraryNode::LocalWriteMake(out);

  // Syntax check.
  string sources = strings::JoinAll(sources_, " ");
  out->StartRule(touchfile_.path(), sources);
  out->WriteCommand("mkdir -p " + ObjectDir() +
                    "; python -m py_compile " + sources +
                    " && touch " + Touchfile().path());
  out->FinishRule();

  // User target.
  if (write_user_target) {
    ResourceFileSet deps;
    DependencyFiles(&deps);
    WriteBaseUserTarget(deps, out);
  }
}

void PyLibraryNode::LocalDependencyFiles(ResourceFileSet* files) const {
  SimpleLibraryNode::LocalDependencyFiles(files);
  files->Add(touchfile_);
}

void PyLibraryNode::Set(const vector<Resource>& sources) {
  sources_ = sources;
  Init();
}

void PyLibraryNode::Init() {
  touchfile_ = Touchfile();
}

}  // namespace repobuild
