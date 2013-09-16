// Copyright 2013
// Author: Christopher Van Arsdale
//
// TODO(cvanarsdale): This overalaps a lot with go_library.

#include <set>
#include <string>
#include <vector>
#include "common/strings/path.h"
#include "common/strings/strutil.h"
#include "repobuild/env/input.h"
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

  // Move all go code into a single directory.
  vector<Resource> symlinked_sources;
  for (const Resource& source : sources_) {
    Resource symlink = PyFileFor(source);
    symlinked_sources.push_back(symlink);
    out->WriteRootSymlink(symlink.path(), source.path());
  }

  // Syntax check.
  string sources = strings::JoinAll(symlinked_sources, " ");
  Makefile::Rule* rule = out->StartRule(touchfile_.path(), sources);
  rule->WriteUserEcho("Compiling", target().full_path() + " (python)");
  rule->WriteCommand("python -m py_compile " + sources);
  rule->WriteCommand("mkdir -p " + Touchfile().dirname());
  rule->WriteCommand("touch " + Touchfile().path());
  out->FinishRule(rule);

  // User target.
  if (write_user_target) {
    ResourceFileSet deps;
    DependencyFiles(PYTHON, &deps);
    WriteBaseUserTarget(deps, out);
  }
}

void PyLibraryNode::LocalDependencyFiles(LanguageType lang,
                                         ResourceFileSet* files) const {
  SimpleLibraryNode::LocalDependencyFiles(lang, files);
  files->Add(touchfile_);
}

void PyLibraryNode::LocalObjectFiles(LanguageType lang,
                                     ResourceFileSet* files) const {
  for (const Resource& r : sources_) {
    files->Add(PyFileFor(r));
  }
}

void PyLibraryNode::Set(const vector<Resource>& sources) {
  sources_ = sources;
  Init();
}

void PyLibraryNode::Init() {
  touchfile_ = Touchfile();
}

Resource PyLibraryNode::PyFileFor(const Resource& r) const {
  return Resource::FromLocalPath(input().pkgfile_dir(),
                                 StripSpecialDirs(r.path()));
}

}  // namespace repobuild
