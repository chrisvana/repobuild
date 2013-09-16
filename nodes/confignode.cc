// Copyright 2013
// Author: Christopher Van Arsdale

#include <set>
#include <string>
#include <vector>
#include "common/log/log.h"
#include "common/strings/path.h"
#include "repobuild/env/input.h"
#include "repobuild/env/resource.h"
#include "repobuild/nodes/confignode.h"
#include "repobuild/reader/buildfile.h"

using std::set;
using std::string;
using std::vector;

namespace repobuild {

void ConfigNode::Parse(BuildFile* file, const BuildFileNode& input) {
  Node::Parse(file, input);
  if (!current_reader()->ParseStringField("component", &component_src_)) {
    LOG(FATAL) << "Could not parse \"component\" in "
               << target().dir() << " config node.";
  }

  file->AddBaseDependency(target().full_path());
  current_reader()->ParseStringField("component_root", &component_root_);
  source_dummy_file_ =
      Resource::FromRootPath(DummyFile(SourceDir("")));
  gendir_dummy_file_ =
      Resource::FromRootPath(DummyFile(SourceDir(Node::input().genfile_dir())));
  pkgfile_dummy_file_ =
      Resource::FromRootPath(DummyFile(SourceDir(Node::input().pkgfile_dir())));
}

void ConfigNode::LocalWriteMake(Makefile* out) const {
  if (component_src_.empty()) {
    return;
  }

  string actual_dir = strings::JoinPath(target().dir(), component_root_);

  // 3 Rules:
  // 1) Our .gen-src directory
  // 2) Our .gen-src/.gen-pkg directory (HACK).
  // 3) .gen-src/.gen-files directory (HACK).
  // 4) User target that generates all 3 above.
  ResourceFileSet dirs;

  {  // (1) .gen-src symlink
    Resource dir = Resource::FromRootPath(source_dummy_file_.dirname());
    dirs.Add(dir);
    AddSymlink(dir.path(), actual_dir, out);
  }

  {  // (2) .gen-src/.gen-pkg symlink
    Resource dir = Resource::FromRootPath(pkgfile_dummy_file_.dirname());
    dirs.Add(dir);
    AddSymlink(dir.path(),
               strings::JoinPath(input().pkgfile_dir(), actual_dir),
               out);
  }

  {  // (3) .gen-src/.gen-files symlink
    Resource dir = Resource::FromRootPath(gendir_dummy_file_.dirname());
    dirs.Add(dir);
    AddSymlink(dir.path(),
               strings::JoinPath(input().genfile_dir(), actual_dir),
               out);
  }

  // (4) User target.
  WriteBaseUserTarget(dirs, out);
}

void ConfigNode::AddSymlink(const string& dir,
                            const string& source,
                            Makefile* out) const {
  // Output link target.
  string link = strings::JoinPath(
      strings::Repeat("../",
                      strings::NumPathComponents(strings::PathDirname(dir))),
      source);

  // Write symlink.
  out->StartRule(dir);
  out->WriteCommand(strings::Join(
      "mkdir -p ", strings::PathDirname(dir), "; ",
      "[ -f ", source, " ] || mkdir -p ", source, "; ",
      "ln -f -s ", link, " ", dir));
  out->FinishRule();

  // Dummy file (to avoid directory timestamp causing everything to rebuild).
  // .gen-src/repobuild/.dummy: .gen-src/repobuild
  //   [ -f .gen-src/repobuild/.dummy ] || touch .gen-src/repobuild/.dummy
  string dummy = DummyFile(dir);
  out->StartRule(dummy, dir);
  out->WriteCommand(strings::Join("[ -f ", dummy, " ] || touch ", dummy));
  out->FinishRule();
}

void ConfigNode::LocalWriteMakeClean(Makefile* out) const {
  if (component_src_.empty()) {
    return;
  }

  out->WriteCommand("rm -rf " + source_dummy_file_.path());
  out->WriteCommand("rm -rf " + gendir_dummy_file_.path());
  out->WriteCommand("rm -rf " + pkgfile_dummy_file_.path());
}

void ConfigNode::LocalDependencyFiles(LanguageType lang,
                                      ResourceFileSet* files) const {
  if (!component_src_.empty()) {
    files->Add(source_dummy_file_);
    files->Add(gendir_dummy_file_);
    files->Add(pkgfile_dummy_file_);
  }
}

void ConfigNode::LocalIncludeDirs(LanguageType lang, set<string>* dirs) const {
  if (!component_src_.empty()) {
    dirs->insert(strings::JoinPath(input().source_dir(),
                                   input().genfile_dir()));
  }
}

string ConfigNode::DummyFile(const string& dir) const {
  return MakefileEscape(strings::JoinPath(dir, ".dummy"));
}

string ConfigNode::SourceDir(const string& middle) const {
  if (middle.empty()) {
    return MakefileEscape(strings::JoinPath(input().source_dir(),
                                            component_src_));
  }
  return MakefileEscape(strings::JoinPath(
      input().source_dir(),
      strings::JoinPath(middle, component_src_)));
}

string ConfigNode::CurrentDir(const string& middle) const {
  if (middle.empty()) {
    return strings::JoinPath(target().dir(), component_src_);
  }
  return strings::JoinPath(target().dir(),
                           strings::JoinPath(middle, component_src_));
}

}  // namespace repobuild
