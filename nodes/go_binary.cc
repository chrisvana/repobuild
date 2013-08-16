// Copyright 2013
// Author: Christopher Van Arsdale

#include <set>
#include <string>
#include <vector>
#include <iterator>
#include "common/log/log.h"
#include "common/strings/path.h"
#include "repobuild/env/input.h"
#include "nodes/go_binary.h"
#include "repobuild/reader/buildfile.h"

using std::string;
using std::vector;
using std::set;

namespace repobuild {

void GoBinaryNode::Parse(BuildFile* file, const BuildFileNode& input) {
  GoLibraryNode::Parse(file, input);
  ParseRepeatedString(input, "go_build_args", &go_build_args_);
}

void GoBinaryNode::WriteMakefile(const vector<const Node*>& all_deps,
                                 string* out) const {
  GoLibraryNode::WriteMakefile(all_deps, out);

  // Source files.
  set<string> source_files;
  CollectDependencies(all_deps, &source_files);

  // Output binary
  string bin = strings::JoinPath(
      strings::JoinPath(input().object_dir(), target().dir()),
      target().local_path());
  out->append(bin + ":");
  for (const string& input : source_files) {
    out->append(" ");
    out->append(input);
  }
  out->append("\n\t");
  out->append("@echo Go build: ");
  out->append(bin);
  out->append("\n\t");
  out->append("@mkdir -p ");
  out->append(strings::PathDirname(bin));
  out->append("; go build -o ");
  out->append(bin);
  for (const string& flag : input().flags("-G")) {
    out->append(" ");
    out->append(flag);
  }
  for (int i = 0; i < go_build_args_.size(); ++i) {
    out->append(" ");
    out->append(go_build_args_[i]);
  }
  for (const string& input : source_files) {
    out->append(" ");
    out->append(input);
  }
  out->append("\n\n");

  // Symlink to root dir.
  string out_bin = OutBinary();
  out->append(out_bin);
  out->append(": ");
  out->append(bin);
  out->append("\n\t");
  out->append("@pwd > /dev/null");  // hack to work around make issue?
  out->append("\n\t@ln -f -s ");
  out->append(strings::JoinPath(input().object_dir(), target().make_path()));
  out->append(" ");
  out->append(out_bin);
  out->append("\n\n");
}

void GoBinaryNode::WriteMakeClean(std::string* out) const {
  out->append("\trm -f ");
  out->append(OutBinary());
  out->append("\n");
}

void GoBinaryNode::FinalOutputs(vector<string>* outputs) const {
  outputs->push_back(OutBinary());
}

std::string GoBinaryNode::OutBinary() const {
  return strings::JoinPath(input().root_dir(), target().local_path());
}

}  // namespace repobuild
