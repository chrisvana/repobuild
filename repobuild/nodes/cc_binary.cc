// Copyright 2013
// Author: Christopher Van Arsdale
//
// TODO(cvanarsdale): This overalaps a lot with other *_binary.cc files.

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

void CCBinaryNode::LocalWriteMake(Makefile* out) const {
  CCLibraryNode::LocalWriteMakeInternal(false, out);

  // Output binary
  Resource bin = ObjBinary();
  WriteLink(bin, out);

  // Symlink to root dir.
  out->WriteRootSymlink(OutBinary().path(), bin.path());

  // Output user target if necessary
  if (OutBinary().path() != target().make_path()) {
    ResourceFileSet deps;
    deps.Add(bin);
    WriteBaseUserTarget(deps, out);
  }
}

void CCBinaryNode::WriteLink(const Resource& file, Makefile* out) const {
  ResourceFileSet objects;
  ObjectFiles(CPP, &objects);

  set<string> flags;
  LinkFlags(CPP, &flags);

  // Link rule
  Makefile::Rule* rule = out->StartRule(file.path(),
                                        strings::JoinAll(objects.files(), " "));
  rule->WriteUserEcho("Linking", file.path());
  string obj_list;
  for (const Resource& r : objects.files()) {
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
  rule->WriteCommand(strings::JoinWith(
      " ",
      "$(LINK.cc)", obj_list, "-o", file,
      strings::JoinAll(flags, " ")));
  out->FinishRule(rule);
}

void CCBinaryNode::LocalWriteMakeClean(Makefile::Rule* rule) const {
  rule->MaybeRemoveSymlink(OutBinary().path());
}

void CCBinaryNode::LocalDependencyFiles(LanguageType lang,
                                        ResourceFileSet* files) const {
  CCLibraryNode::LocalDependencyFiles(lang, files);
  LocalBinaries(lang, files);
}

void CCBinaryNode::LocalFinalOutputs(LanguageType lang,
                                     ResourceFileSet* outputs) const {
  CCLibraryNode::LocalFinalOutputs(lang, outputs);
  outputs->Add(OutBinary());
}

void CCBinaryNode::LocalBinaries(LanguageType lang,
                                 ResourceFileSet* outputs) const {
  outputs->Add(ObjBinary());
}

Resource CCBinaryNode::OutBinary() const {
  return Resource::FromLocalPath(input().root_dir(), target().local_path());
}

Resource CCBinaryNode::ObjBinary() const {
  return Resource::FromLocalPath(input().object_dir(), target().make_path());
}

}  // namespace repobuild
