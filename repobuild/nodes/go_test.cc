// Copyright 2013
// Author: Christopher Van Arsdale
//
// TODO(cvanarsdale): This overalaps a lot with go_binary.cc.

#include <set>
#include <string>
#include <vector>
#include <iterator>
#include "common/log/log.h"
#include "common/strings/path.h"
#include "repobuild/env/input.h"
#include "repobuild/nodes/go_test.h"
#include "repobuild/reader/buildfile.h"

using std::string;
using std::vector;
using std::set;

namespace repobuild {

void GoTestNode::Parse(BuildFile* file, const BuildFileNode& input) {
  GoLibraryNode::Parse(file, input);
  current_reader()->ParseRepeatedString("go_build_args", &go_build_args_);
  if (sources_.empty()) {
    LOG(FATAL) << "go_test requires \"go_sources\" to be non-empty: "
               << target().full_path();
  }
}

void GoTestNode::LocalWriteMake(Makefile* out) const {
  GoLibraryNode::LocalWriteMakeInternal(false, out);
  WriteGoTest(out);
  ResourceFileSet files;
  files.Add(Touchfile("test"));
  WriteBaseUserTarget(files, out);
}

void GoTestNode::LocalTests(LanguageType lang,
                            set<string>* targets) const {
  targets->insert(target().make_path());
}

void GoTestNode::WriteGoTest(Makefile* out) const {
  // Source files.
  ResourceFileSet deps;
  DependencyFiles(GO_LANG, &deps);

  ResourceFileSet inputs;
  LocalObjectFiles(GO_LANG, &inputs);

  // Output test
  Resource touchfile = Touchfile("test");
  Makefile::Rule* rule = out->StartRule(touchfile.path(),
                                        strings::JoinAll(deps.files(), " "));
  rule->WriteUserEcho("Testing", target().make_path());
  rule->WriteCommand(
      strings::JoinWith(
          " ",
          GoBuildPrefix(), "go test",
          strings::JoinAll(input().flags("-G"), " "),
          strings::JoinAll(go_build_args_, " "),
          strings::JoinAll(inputs.files(), " ")));
  rule->WriteCommand("mkdir -p " + touchfile.dirname());
  rule->WriteCommand("touch " + touchfile.path());
  out->FinishRule(rule);
}

}  // namespace repobuild
