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
  WriteGoBinary(Binary(), out);
  WriteBaseUserTarget(out);
}

void GoBinaryNode::WriteGoBinary(const Resource& bin, Makefile* out) const {
  // Source files.
  ResourceFileSet deps;
  DependencyFiles(GO_LANG, &deps);

  ResourceFileSet inputs;
  LocalObjectFiles(GO_LANG, &inputs);

  // Output binary
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
}

void GoBinaryNode::LocalBinaries(LanguageType lang,
                                 ResourceFileSet* outputs) const {
  outputs->Add(Binary());
}

Resource GoBinaryNode::Binary() const {
  return Resource::FromLocalPath(input().object_dir(), target().make_path());
}

}  // namespace repobuild
