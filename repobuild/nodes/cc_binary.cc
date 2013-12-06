// Copyright 2013
// Author: Christopher Van Arsdale
//
// TODO(cvanarsdale): This overalaps with cc_shared_library.

#include <algorithm>
#include <set>
#include <string>
#include <vector>
#include <iterator>
#include "common/log/log.h"
#include "common/strings/path.h"
#include "repobuild/env/input.h"
#include "repobuild/nodes/cc_binary.h"
#include "repobuild/nodes/top_symlink.h"
#include "repobuild/reader/buildfile.h"

using std::string;
using std::vector;
using std::set;

namespace repobuild {

void CCBinaryNode::Parse(BuildFile* file, const BuildFileNode& input) {
  CCLibraryNode::Parse(file, input);
  ResourceFileSet binaries;
  LocalBinaries(NO_LANG, &binaries);
  AddSubNode(new TopSymlinkNode(
      GetNextTargetName(file),
      Node::input(),
      Node::dist_source(),
      binaries));
}

void CCBinaryNode::LocalWriteMake(Makefile* out) const {
  CCLibraryNode::LocalWriteMakeInternal(false, out);
  WriteLink(ObjBinary(), out);
  WriteBaseUserTarget(out);
}

/**
 * Adds to "has_tag" each Resource in "fileset" that has "tag".
 * Adds to "no_tag" each Resource in "fileset" that does not have "tag".
 */
static void Partition(const ResourceFileSet& fileset,
		      const string& tag,
		      ResourceFileSet* has_tag,
		      ResourceFileSet* no_tag) {
  for (auto it : fileset) {
    if (it.has_tag(tag)) {
      has_tag->Add(it);
    } else {
      no_tag->Add(it);
    }
  }
}

void CCBinaryNode::WriteLink(const Resource& file, Makefile* out) const {
  ResourceFileSet objects;
  ObjectFiles(CPP, &objects);

  ResourceFileSet ephemeral_objects, normal_objects;
  Partition(objects, "ephemeral", &ephemeral_objects, &normal_objects);

  set<string> flags;
  LinkFlags(CPP, &flags);

  // Link rule
  Makefile::Rule* rule =
    out->StartRule(file.path(), strings::JoinAll(normal_objects.files(), " "));

  string ephemeral_targets = strings::JoinAll(ephemeral_objects.files(), " ");
  if (!ephemeral_targets.empty()) {
    rule->WriteUserEcho("Making ephemeral inputs", ephemeral_targets);
    rule->WriteCommand("make " + ephemeral_targets);
  }

  rule->WriteUserEcho("Linking", file.path());

  // HACK(cvanarsdale):
  // Sadly order matters to the (gcc) linker. It looks in later object
  // files to find unresolved symbols. We collect the dependencies
  // bottom up, so we push resources onto the front of the list so
  // unencumbered resources end up in the back of the list.
  string obj_list;
  vector<Resource> copy = objects.files();
  std::reverse(copy.begin(), copy.end());
  for (const Resource& r : copy) {
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
  rule->WriteCommand("mkdir -p " + file.dirname());
  rule->WriteCommand(strings::JoinWith(
      " ",
      "$(LINK.cc)", obj_list, "-o", file,
      strings::JoinAll(flags, " ")));
  out->FinishRule(rule);
}

void CCBinaryNode::LocalWriteMakeInstall(Makefile* base,
                                         Makefile::Rule* rule) const {
  rule->AddDependency(ObjBinary().basename());
  rule->WriteCommand("mkdir -p $(DESTDIR)$(bindir)");
  rule->WriteCommand("$(INSTALL_PROGRAM) " +
                     ObjBinary().path() + " $(DESTDIR)$(bindir)/" +
                     ObjBinary().basename());
}

void CCBinaryNode::LocalBinaries(LanguageType lang,
                                 ResourceFileSet* outputs) const {
  outputs->Add(ObjBinary());
}

Resource CCBinaryNode::ObjBinary() const {
  return Resource::FromLocalPath(input().object_dir(), target().make_path());
}

bool CCBinaryNode::ShouldInclude(DependencyCollectionType type,
                                 LanguageType lang) const {
  return (type != OBJECT_FILES &&
          type != LINK_FLAGS &&
          type != COMPILE_FLAGS &&
          type != INCLUDE_DIRS);
}


}  // namespace repobuild
