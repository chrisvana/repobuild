// Copyright 2013
// Author: Christopher Van Arsdale

#include <string>
#include <vector>
#include "common/log/log.h"
#include "common/strings/path.h"
#include "repobuild/env/input.h"
#include "nodes/confignode.h"
#include "repobuild/reader/buildfile.h"

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
                               string* out) const {
  if (component_src_.empty()) {
    return;
  }

  // This is the directory we create.
  string dir = SourceDir("");
  out->append(dir);
  out->append(":\n");

  // ... And this is how we create it:
  // mkdir -p src; ln -s path/to/this/target/dir src/<component>
  AddSymlink(dir, strings::JoinPath(target().dir(), component_root_), out);


  // Same thing for genfiles
  dir = SourceDir(input().genfile_dir());
  out->append(dir);
  out->append(":\n");
  AddSymlink(dir,
             strings::JoinPath(input().genfile_dir(),
                               strings::JoinPath(target().dir(),
                                                 component_root_)),
             out);
}

void ConfigNode::AddSymlink(const string& dir,
                            const string& source,
                            string* out) const {
  out->append("\t");
  out->append("mkdir -p ");
  out->append(strings::PathDirname(dir));
  out->append("; if [[ ! -a ");
  out->append(source);
  out->append(" ]]; then mkdir -p ");
  out->append(source);
  out->append("; fi; ln -f -s ");
  int num_pieces = strings::NumPathComponents(strings::PathDirname(dir));
  string link;
  for (int i = 0; i < num_pieces; ++i) {
    link += "../";
  }
  out->append(strings::JoinPath(link, source));
  out->append(" ");
  out->append(dir);
  out->append("\n\n");

  // Dummy file (to avoid directory timestamp causing everything to rebuild).
  // src/repobuild/.dummy: src/repobuild
  //   if [[ ! -a src/repobuild/.dummy ]]; then touch src/repobuild/.dummy; fi
  string dummy = DummyFile(dir);
  out->append(dummy);
  out->append(": ");
  out->append(dir);  // input
  out->append("\n\tif [[ ! -a ");
  out->append(dummy);
  out->append(" ]]; then touch ");
  out->append(dummy);
  out->append("; fi\n\n");
}

void ConfigNode::WriteMakeClean(std::string* out) const {
  if (component_src_.empty()) {
    return;
  }

  out->append("\trm -f ");
  out->append(DummyFile(SourceDir("")));
  out->append("\n\trm -f ");
  out->append(SourceDir(SourceDir("")));
  out->append("\n");

  out->append("\trm -f ");
  out->append(DummyFile(SourceDir(input().genfile_dir())));
  out->append("\n\trm -f ");
  out->append(SourceDir(input().genfile_dir()));
  out->append("\n");
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
