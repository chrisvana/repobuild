// Copyright 2013
// Author: Christopher Van Arsdale

#include <set>
#include <string>
#include <vector>
#include <iterator>
#include "common/log/log.h"
#include "common/strings/path.h"
#include "repobuild/env/input.h"
#include "repobuild/nodes/go_binary.h"
#include "repobuild/reader/buildfile.h"

using std::string;
using std::vector;
using std::set;

namespace repobuild {

void GoBinaryNode::Parse(BuildFile* file, const BuildFileNode& input) {
  GoLibraryNode::Parse(file, input);
  current_reader()->ParseRepeatedString("go_build_args", &go_build_args_);
  if (sources_.empty()) {
    LOG(FATAL) << "go_binary requires \"go_sources\" to be non-empty: "
               << target().full_path();
  }
}

void GoBinaryNode::LocalWriteMake(Makefile* out) const {
  GoLibraryNode::LocalWriteMakeInternal(false, out);

  // Source files.
  ResourceFileSet deps;
  DependencyFiles(GOLANG, &deps);

  ResourceFileSet inputs;
  LocalObjectFiles(GOLANG, &inputs);

  // Output binary
  Resource bin = Binary();
  out->StartRule(bin.path(), strings::JoinAll(deps.files(), " "));
  out->WriteCommand("echo Go build: " + bin.path());
  out->WriteCommand("mkdir -p " + bin.dirname());
  out->WriteCommand(
      strings::JoinWith(
          " ",
          GoBuildPrefix(), "go build -o", bin,
          strings::JoinAll(input().flags("-G"), " "),
          strings::JoinAll(go_build_args_, " "),
          strings::JoinAll(inputs.files(), " ")));
  out->FinishRule();

  // User target
  {
    ResourceFileSet bins;
    bins.Add(bin);
    WriteBaseUserTarget(bins, out);
  }

  // Symlink to root dir.
  Resource out_bin = OutBinary();
  out->StartRule(out_bin.path(), bin.path());
  out->WriteCommand("pwd > /dev/null");  // hack to work around make issue?
  out->WriteCommand(
      strings::JoinWith(
          " ",
          "ln -f -s",
          strings::JoinPath(input().object_dir(), target().make_path()),
          out_bin.path()));
  out->FinishRule();
}

void GoBinaryNode::LocalWriteMakeClean(Makefile* out) const {
  out->WriteCommand("rm -f " + OutBinary().path());
}

void GoBinaryNode::LocalFinalOutputs(LanguageType lang,
                                     ResourceFileSet* outputs) const {
  Node::LocalFinalOutputs(lang, outputs);
  outputs->Add(OutBinary());
}

void GoBinaryNode::LocalBinaries(LanguageType lang,
                                 ResourceFileSet* outputs) const {
  outputs->Add(Binary());
}

Resource GoBinaryNode::OutBinary() const {
  return Resource::FromLocalPath(input().root_dir(), target().local_path());
}

Resource GoBinaryNode::Binary() const {
  return Resource::FromLocalPath(
      strings::JoinPath(input().object_dir(), target().dir()),
      target().local_path());
}

}  // namespace repobuild
