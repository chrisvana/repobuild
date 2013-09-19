// Copyright 2013
// Author: Christopher Van Arsdale
//
// TODO(cvanarsdale): This overalaps a lot with py_library.

#include <set>
#include <string>
#include <vector>
#include "common/strings/strutil.h"
#include "common/strings/path.h"
#include "repobuild/env/input.h"
#include "repobuild/nodes/go_library.h"
#include "repobuild/reader/buildfile.h"

using std::vector;
using std::string;
using std::set;

namespace repobuild {

void GoLibraryNode::Parse(BuildFile* file, const BuildFileNode& input) {
  Node::Parse(file, input);

  // go_sources
  current_reader()->ParseRepeatedFiles("go_sources", &sources_);

  Init();
}

void GoLibraryNode::Init() {
  touchfile_ = Touchfile();
}

void GoLibraryNode::Set(const vector<Resource>& sources) {
  sources_ = sources;
  Init();
}

void GoLibraryNode::LocalWriteMakeInternal(bool write_user_target,
                                           Makefile* out) const {
  // Move all go code into a single directory.
  vector<Resource> symlinked_sources;
  for (const Resource& source : sources_) {
    Resource symlink = GoFileFor(source);
    symlinked_sources.push_back(symlink);
    out->WriteRootSymlink(symlink.path(), source.path());
  }

  // Syntax check.
  Makefile::Rule* rule = out->StartRule(
      touchfile_.path(),
      strings::JoinAll(symlinked_sources, " "));
  rule->WriteUserEcho("Compiling", target().full_path() + " (go)");
  if (!sources_.empty()) {
    for (const Resource& r : symlinked_sources) {
      rule->WriteCommand(GoBuildPrefix() + " gofmt " +
                        r.path() + " > /dev/null");
    }
  }
  rule->WriteCommand("mkdir -p " + Touchfile().dirname());
  rule->WriteCommand("touch " + Touchfile().path());
  out->FinishRule(rule);

  // User target.
  if (write_user_target) {
    ResourceFileSet deps;
    DependencyFiles(GO_LANG, &deps);
    WriteBaseUserTarget(deps, out);
  }
}

void GoLibraryNode::LocalDependencyFiles(LanguageType lang,
                                         ResourceFileSet* files) const {
  files->AddRange(sources_);
  files->Add(touchfile_);
}

void GoLibraryNode::LocalObjectFiles(LanguageType lang,
                                     ResourceFileSet* files) const {
  for (const Resource& r : sources_) {
    files->Add(GoFileFor(r));
  }
}

Resource GoLibraryNode::GoFileFor(const Resource& r) const {
  // HACK, go annoys me.
  string path = StripSpecialDirs(r.path());
  if (strings::HasPrefix(path, "src/")) {
    return Resource::FromLocalPath(input().pkgfile_dir(), path);
  } else {
    return Resource::FromLocalPath(input().pkgfile_dir() + "/src", path);
  }
}

string GoLibraryNode::GoBuildPrefix() const {
  return MakefileEscape("GOPATH=$(pwd)/" + input().pkgfile_dir() + ":$GOPATH");
}

}  // namespace repobuild
