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
                                 Makefile* out) const {
  GoLibraryNode::WriteMakefile(all_deps, out);

  // Source files.
  set<string> source_files;
  CollectDependencies(all_deps, &source_files);

  // Output binary
  string bin = strings::JoinPath(
      strings::JoinPath(input().object_dir(), target().dir()),
      target().local_path());
  out->StartRule(bin, strings::Join(source_files, " "));
  out->WriteCommand("echo Go build: " + bin);
  out->WriteCommand("mkdir -p " + strings::PathDirname(bin));
  out->WriteCommand(
      strings::JoinAllWith(
          " ",
          "go build -o", bin,
          strings::Join(input().flags("-G"), " "),
          strings::Join(go_build_args_, " "),
          strings::Join(source_files, " ")));
  out->FinishRule();

  // Symlink to root dir.
  string out_bin = OutBinary();
  out->StartRule(out_bin, bin);
  out->WriteCommand("pwd > /dev/null");  // hack to work around make issue?
  out->WriteCommand(
      strings::JoinAllWith(
          " ",
          "ln -f -s",
          strings::JoinPath(input().object_dir(), target().make_path()),
          out_bin));
  out->FinishRule();
}

void GoBinaryNode::WriteMakeClean(Makefile* out) const {
  out->WriteCommand("rm -f " + OutBinary());
}

void GoBinaryNode::FinalOutputs(vector<string>* outputs) const {
  outputs->push_back(OutBinary());
}

std::string GoBinaryNode::OutBinary() const {
  return strings::JoinPath(input().root_dir(), target().local_path());
}

}  // namespace repobuild
