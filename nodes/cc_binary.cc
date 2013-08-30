// Copyright 2013
// Author: Christopher Van Arsdale

#include <algorithm>
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
  Resource bin = Resource::FromLocalPath(input().object_dir(),
                                         target().make_path());
  WriteLink(all_deps, bin, out);

  {  // Output user target
    set<Resource> deps;
    deps.insert(bin);
    WriteBaseUserTarget(deps, out);
  }

  // Symlink to root dir.
  Resource out_bin = OutBinary();
  out->StartRule(out_bin.path(), bin.path());
  out->WriteCommand("pwd > /dev/null");  // hack to work around make issue?
  out->WriteCommand(
      strings::Join(
          "ln -f -s ",
          strings::JoinPath(input().object_dir(), target().make_path()),
          " ", out_bin.path()));
  out->FinishRule();
}

void CCBinaryNode::WriteLink(
    const vector<const Node*>& all_deps,
    const Resource& file,
    Makefile* out) const {
  vector<Resource> objects;
  CollectObjects(all_deps, &objects);

  set<string> flags;
  CollectLinkFlags(all_deps, &flags);

  // HACK(cvanarsdale):
  // Sadly order matters to the (gcc) linker. It looks in later object files
  // to find unresolved symbols. Since we collected the dependencies in order,
  // we get the objects in order too (a <- b <- c, etc). We want c b a in the
  // output, so "a" is last. Thus, we reverse the list.... shoot me.
  std::reverse(objects.begin(), objects.end());

  // Link rule
  out->StartRule(file.path(), strings::JoinAll(objects, " "));
  out->WriteCommand("echo Linking: " + file.path());
  string obj_list;
  for (const Resource& r : objects) {
    obj_list += " ";
    bool alwayslink = r.has_tag("alwayslink");
    if (alwayslink) {
      obj_list += "$(LD_FORCE_LINK_START) ";
    }
    obj_list += r.path();
    if (alwayslink) {
      obj_list += " $(LD_FORCE_LINK_END)";
    }
  }
  out->WriteCommand(strings::JoinWith(
      " ",
      "$(LINK.cc)", obj_list, "-o", file,
      strings::JoinAll(flags, " ")));
  out->FinishRule();
}

void CCBinaryNode::WriteMakeClean(const vector<const Node*>& all_deps,
                                  Makefile* out) const {
  out->WriteCommand("rm -f " + OutBinary().path());
}

void CCBinaryNode::FinalOutputs(vector<Resource>* outputs) const {
  outputs->push_back(OutBinary());
}

Resource CCBinaryNode::OutBinary() const {
  return Resource::FromLocalPath(input().root_dir(), target().local_path());
}

}  // namespace repobuild
