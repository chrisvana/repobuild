// Copyright 2013
// Author: Christopher Van Arsdale

#include <set>
#include <string>
#include <vector>
#include <iterator>
#include "common/log/log.h"
#include "common/strings/path.h"
#include "repobuild/env/input.h"
#include "repobuild/nodes/cc_binary.h"
#include "repobuild/reader/buildfile.h"

using std::string;
using std::vector;
using std::set;

namespace repobuild {

void CCBinaryNode::Parse(BuildFile* file, const BuildFileNode& input) {
  CCLibraryNode::Parse(file, input);
}

void CCBinaryNode::WriteMakefile(const vector<const Node*>& all_deps,
                                 Makefile* out) const {
  CCLibraryNode::WriteMakefileInternal(all_deps, false, out);

  // Output binary
  string bin = strings::JoinPath(input().object_dir(), target().make_path());
  WriteLink(all_deps, bin, out);

  {  // Output user target
    set<string> deps;
    deps.insert(bin);
    WriteBaseUserTarget(deps, out);
  }

  // Symlink to root dir.
  string out_bin = OutBinary();
  out->StartRule(out_bin, bin);
  out->WriteCommand("pwd > /dev/null");  // hack to work around make issue?
  out->WriteCommand(
      strings::Join(
          "ln -f -s ",
          strings::JoinPath(input().object_dir(), target().make_path()),
          " ", out_bin));
  out->FinishRule();
}

void CCBinaryNode::WriteLink(
    const vector<const Node*>& all_deps,
    const string& file,
    Makefile* out) const {
  set<string> objects;
  CollectObjects(all_deps, &objects);

  set<string> flags;
  CollectLinkFlags(all_deps, &flags);

  string list = strings::JoinAll(objects, " ");

  // Link rule
  out->StartRule(file, list);
  out->WriteCommand("echo Linking: " + file);
  out->WriteCommand(strings::JoinWith(
      " ",
      "$(LINK.cc)", list, "-o", file,
      strings::JoinAll(flags, " ")));
  out->FinishRule();
}

void CCBinaryNode::WriteMakeClean(Makefile* out) const {
  out->WriteCommand("rm -f " + OutBinary());
}

void CCBinaryNode::FinalOutputs(vector<string>* outputs) const {
  outputs->push_back(OutBinary());
}

std::string CCBinaryNode::OutBinary() const {
  return strings::JoinPath(input().root_dir(), target().local_path());
}

}  // namespace repobuild
