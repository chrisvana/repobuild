// Copyright 2013
// Author: Christopher Van Arsdale

#include <set>
#include <string>
#include <vector>
#include <iterator>
#include "repobuild/env/input.h"
#include "repobuild/env/resource.h"
#include "repobuild/nodes/py_binary.h"
#include "repobuild/nodes/top_symlink.h"
#include "repobuild/reader/buildfile.h"

using std::string;
using std::vector;
using std::set;

namespace repobuild {

void PyBinaryNode::Parse(BuildFile* file, const BuildFileNode& input) {
  PyEggNode::Parse(file, input);
  ResourceFileSet binaries;
  LocalBinaries(NO_LANG, &binaries);
  AddSubNode(new TopSymlinkNode(
      target().GetParallelTarget(file->NextName(target().local_path())),
      Node::input(),
      Node::dist_source(),
      binaries));
}

void PyBinaryNode::LocalWriteMake(Makefile* out) const {
  PyEggNode::LocalWriteMakeInternal(false, out);

  // "Binary"
  Resource bin = BinScript();
  Resource egg = OutEgg();
  Makefile::Rule* rule = out->StartRule(bin.path(), egg.path());
  string module = py_default_module_.empty() ? "" : "-m " + py_default_module_;
  rule->WriteCommand("echo 'PYTHONPATH=$$(pwd)/$$(dirname $$0)/" +
                     egg.basename() +":$$PYTHONPATH python " + module +
                     "' > " + bin.path() +
                     "; chmod 755 " + bin.path());
  out->FinishRule(rule);

  WriteBaseUserTarget(out);
}

void PyBinaryNode::LocalBinaries(LanguageType lang,
                                 ResourceFileSet* outputs) const {
  outputs->Add(BinScript());
  outputs->Add(OutEgg());
}

Resource PyBinaryNode::BinScript() const {
  return Resource::FromLocalPath(input().object_dir(), target().make_path());
}

bool PyBinaryNode::ShouldInclude(DependencyCollectionType type,
                                 LanguageType lang) const {
  return (type != OBJECT_FILES &&
          type != LINK_FLAGS &&
          type != COMPILE_FLAGS &&
          type != INCLUDE_DIRS);
}

}  // namespace repobuild
