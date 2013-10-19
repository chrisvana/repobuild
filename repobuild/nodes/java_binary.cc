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
#include "common/strings/strutil.h"
#include "repobuild/env/input.h"
#include "repobuild/nodes/java_binary.h"
#include "repobuild/nodes/top_symlink.h"
#include "repobuild/reader/buildfile.h"

using std::string;
using std::vector;
using std::set;

namespace repobuild {

void JavaBinaryNode::Parse(BuildFile* file, const BuildFileNode& input) {
  JavaJarNode::Parse(file, input);
  
  bool have_main_class = false;
  for (const string& j : java_manifest_) {
    have_main_class |= strings::HasPrefix(j, "Main-Class:");
  }

  if (!have_main_class) {
    if (sources_.size() == 1) {
      string classname = strings::ReplaceAll(sources_[0].path(), "/", ".");
      classname = classname.substr(0, classname.size() - 5);  // remove .java
      java_manifest_.push_back("Main-class: " + classname);
    } else {
      LOG(FATAL) << "java_binary requires Main-class:... in "
                 << "java_binary.java_manifest[].";
    }
  }

  ResourceFileSet binaries;
  LocalBinaries(NO_LANG, &binaries);
  AddSubNode(new TopSymlinkNode(
      target().GetParallelTarget(file->NextName(target().local_path())),
      Node::input(),
      Node::dist_source(),
      binaries));
}

void JavaBinaryNode::LocalWriteMake(Makefile* out) const {
  JavaJarNode::LocalWriteMakeInternal(false, out);

  // "Binary"
  Resource bin = BinScript();
  Resource jar = JarName();
  Makefile::Rule* rule = out->StartRule(bin.path(), jar.path());
  rule->WriteCommand("echo 'java -jar $$(pwd)/$$(dirname $$0)/" +
                     jar.path() + "' > " + bin.path() +
                     "; chmod 755 " + bin.path());
  out->FinishRule(rule);

  WriteBaseUserTarget(out);
}

void JavaBinaryNode::LocalBinaries(LanguageType lang,
                                   ResourceFileSet* outputs) const {
  outputs->Add(BinScript());
  outputs->Add(JarName());
}

Resource JavaBinaryNode::BinScript() const {
  return Resource::FromLocalPath(input().object_dir(), target().make_path());
}

bool JavaBinaryNode::ShouldInclude(DependencyCollectionType type,
                                   LanguageType lang) const {
  return (type != OBJECT_FILES &&
          type != LINK_FLAGS &&
          type != COMPILE_FLAGS &&
          type != INCLUDE_DIRS);
}

}  // namespace repobuild
