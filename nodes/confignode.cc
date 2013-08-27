// Copyright 2013
// Author: Christopher Van Arsdale

#include <set>
#include <string>
#include <vector>
#include "common/log/log.h"
#include "common/strings/path.h"
#include "repobuild/env/input.h"
#include "repobuild/nodes/confignode.h"
#include "repobuild/reader/buildfile.h"

using std::set;
using std::string;
using std::vector;

namespace repobuild {

void ConfigNode::Parse(BuildFile* file, const BuildFileNode& input) {
  Node::Parse(file, input);
  if (ParseStringField(input, "component", &component_src_)) {
    file->AddBaseDependency(target().full_path());
    ParseStringField(input, "component_root", &component_root_);
  } else {
    LOG(FATAL) << "Could not parse \"component\" in "
               << target().dir() << " config node.";
  }
}

void ConfigNode::WriteMakefile(const vector<const Node*>& all_deps,
                               Makefile* out) const {
  if (component_src_.empty()) {
    return;
  }

  // 3 Rules:
  // 1) Our .gen-src directory
  // 2) Our base files (e.g. the .h and .cc files)
  // 3) Our .gen-files directory

  // (1) .gen-src symlink
  string dir = SourceDir("");  // directory we create
  AddSymlink(dir, strings::JoinPath(target().dir(), component_root_), out);

  // (2) Main files
  {
    set<string> targets;
    targets.insert(dir);
    WriteBaseUserTarget(targets, out);
  }

  // (3) .gen-files symlink
  dir = SourceDir(input().genfile_dir());  // directory we create
  AddSymlink(dir,
             strings::JoinPath(input().genfile_dir(),
                               strings::JoinPath(target().dir(),
                                                 component_root_)),
             out);
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

void ConfigNode::WriteMakeClean(Makefile* out) const {
  if (component_src_.empty()) {
    return;
  }

  out->WriteCommand("rm -rf " + DummyFile(SourceDir("")));
  out->WriteCommand("rm -rf " + SourceDir(SourceDir("")));
  out->WriteCommand("rm -rf " + DummyFile(SourceDir(input().genfile_dir())));
  out->WriteCommand("rm -rf " + SourceDir(input().genfile_dir()));
}

void ConfigNode::DependencyFiles(vector<string>* files) const {
  Node::DependencyFiles(files);
  if (!component_src_.empty()) {
    files->push_back(DummyFile(SourceDir("")));
    files->push_back(DummyFile(SourceDir(input().genfile_dir())));
  }
}

std::string ConfigNode::DummyFile(const string& dir) const {
  return MakefileEscape(strings::JoinPath(dir, ".dummy"));
}

std::string ConfigNode::SourceDir(const string& middle) const {
  if (middle.empty()) {
    return MakefileEscape(strings::JoinPath(input().source_dir(),
                                            component_src_));
  }
  return MakefileEscape(strings::JoinPath(
      input().source_dir(),
      strings::JoinPath(middle, component_src_)));
}

std::string ConfigNode::CurrentDir(const string& middle) const {
  if (middle.empty()) {
    return strings::JoinPath(target().dir(), component_src_);
  }
  return strings::JoinPath(target().dir(),
                           strings::JoinPath(middle, component_src_));
}

}  // namespace repobuild
