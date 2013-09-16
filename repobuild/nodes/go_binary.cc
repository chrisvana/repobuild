// Copyright 2013
// Author: Christopher Van Arsdale
//
// TODO(cvanarsdale): This overalaps a lot with other *_binary.cc files.

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
  DependencyFiles(GO_LANG, &deps);

  ResourceFileSet inputs;
  LocalObjectFiles(GO_LANG, &inputs);

  // Output binary
  Resource bin = Binary();
  Makefile::Rule* rule = out->StartRule(bin.path(),
                                        strings::JoinAll(deps.files(), " "));
  rule->WriteUserEcho("Go build", bin.path());
  rule->WriteCommand("mkdir -p " + bin.dirname());
  rule->WriteCommand(
      strings::JoinWith(
          " ",
          GoBuildPrefix(), "go build -o", bin,
          strings::JoinAll(input().flags("-G"), " "),
          strings::JoinAll(go_build_args_, " "),
          strings::JoinAll(inputs.files(), " ")));
  out->FinishRule(rule);

  // Symlink to root dir.
  out->WriteRootSymlink(OutBinary().path(), Binary().path());

  // User target, if necessary.
  if (OutBinary().path() != target().make_path()) {
    ResourceFileSet bins;
    bins.Add(bin);
    WriteBaseUserTarget(bins, out);
  }
}

void GoBinaryNode::LocalWriteMakeClean(Makefile::Rule* rule) const {
  rule->MaybeRemoveSymlink(OutBinary().path());
}

void GoBinaryNode::LocalDependencyFiles(LanguageType lang,
                                        ResourceFileSet* outputs) const {
  GoLibraryNode::LocalDependencyFiles(lang, outputs);
  LocalBinaries(lang, outputs);
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
