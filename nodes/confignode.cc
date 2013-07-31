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
  }
}

void ConfigNode::WriteMakefile(const vector<const Node*>& all_deps,
                               string* out) const {
  if (component_src_.empty()) {
    return;
  }

  // This is the directory we create.
  string dir = SourceDir();
  out->append(dir);
  out->append(":\n");

  // ... And this is how we create it:
  // mkdir -p src; ln -s path/to/this/target/dir src/<component>
  out->append("\t");
  out->append("mkdir -p ");
  out->append(input().source_dir());
  string relative_component = strings::JoinPath(target().dir(),
                                                component_root_);
  out->append("; if [[ ! -a ");
  out->append(relative_component);
  out->append(" ]]; then mkdir -p ");
  out->append(relative_component);
  out->append("; fi; ln -s ");
  out->append(strings::JoinPath(input().full_root_dir(), relative_component));
  out->append(" ");
  out->append(dir);
  out->append("\n\n");

  // Dummy file (to avoid directory timestamp causing everything to rebuild).
  // src/repobuild/.dummy: src/repobuild
  //   if [[ ! -a src/repobuild/.dummy ]]; then touch src/repobuild/.dummy; fi
  string dummy = DummyFile();
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
  out->append(DummyFile());
  out->append("\n\trm -f ");
  out->append(SourceDir());
  out->append("\n");
}

void ConfigNode::DependencyFiles(vector<string>* files) const {
  Node::DependencyFiles(files);
  if (!component_src_.empty()) {
    files->push_back(DummyFile());
  }
}

std::string ConfigNode::DummyFile() const {
  return MakefileEscape(strings::JoinPath(SourceDir(), ".dummy"));
}

std::string ConfigNode::SourceDir() const {
  return MakefileEscape(strings::JoinPath(input().source_dir(),
                                          component_src_));
}

}  // namespace repobuild
