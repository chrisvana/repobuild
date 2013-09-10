// Copyright 2013
// Author: Christopher Van Arsdale

#include <set>
#include <string>
#include <vector>
#include "repobuild/nodes/go_library.h"
#include "repobuild/reader/buildfile.h"

using std::vector;
using std::string;
using std::set;

namespace repobuild {

void GoLibraryNode::Parse(BuildFile* file, const BuildFileNode& input) {
  SimpleLibraryNode::Parse(file, input);

  // go_sources
  current_reader()->ParseRepeatedFiles("go_sources", &sources_);
}

void GoLibraryNode::WriteMakefileInternal(
    const std::vector<const Node*>& all_deps,
    bool write_user_target,
    Makefile* out) const {
  SimpleLibraryNode::WriteMakefile(all_deps, out);

  // Syntax check.
  string sources = strings::JoinAll(sources_, " ");
  out->StartRule(Touchfile().path(), sources);
  out->WriteCommand(
      "mkdir -p " + ObjectDir() +
      "; gofmt -e " + sources + " && touch " + Touchfile().path());
  out->FinishRule();

  // User target.
  if (write_user_target) {
    set<Resource> deps;
    CollectDependencies(all_deps, &deps);
    deps.insert(Touchfile());
    WriteBaseUserTarget(deps, out);
  }
}

void GoLibraryNode::DependencyFiles(vector<Resource>* files) const {
  SimpleLibraryNode::DependencyFiles(files);
  files->push_back(Touchfile());
}

}  // namespace repobuild
