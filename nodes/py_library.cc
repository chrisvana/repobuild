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
}

void PyLibraryNode::WriteMakefileInternal(
    const std::vector<const Node*>& all_deps,
    bool write_user_target,
    Makefile* out) const {
  SimpleLibraryNode::WriteMakefile(all_deps, out);

  // Syntax check.
  string sources = strings::JoinAll(sources_, " ");
  out->StartRule(Touchfile().path(), sources);
  out->WriteCommand("mkdir -p " + ObjectDir() +
                    "; python -m py_compile " + sources +
                    " && touch " + Touchfile().path());
  out->FinishRule();

  // User target.
  if (write_user_target) {
    set<Resource> deps;
    DependencyFiles(&deps);
    WriteBaseUserTarget(deps, out);
  }
}

void PyLibraryNode::DependencyFiles(set<Resource>* files) const {
  SimpleLibraryNode::DependencyFiles(files);
  files->insert(Touchfile());
}

}  // namespace repobuild
