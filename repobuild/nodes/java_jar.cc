// Copyright 2013
// Author: Christopher Van Arsdale
//
// TODO(cvanarsdale): This overalaps a lot with other *_jar.cc files.

#include <algorithm>
#include <set>
#include <string>
#include <vector>
#include <iterator>
#include "common/log/log.h"
#include "common/strings/path.h"
#include "common/strings/strutil.h"
#include "repobuild/env/input.h"
#include "repobuild/nodes/java_jar.h"
#include "repobuild/nodes/top_symlink.h"
#include "repobuild/reader/buildfile.h"

using std::string;
using std::vector;
using std::set;

namespace repobuild {

void JavaJarNode::Parse(BuildFile* file, const BuildFileNode& input) {
  JavaLibraryNode::Parse(file, input);
  
  current_reader()->ParseRepeatedString("java_manifest", &java_manifest_);
}

void JavaJarNode::LocalWriteMake(Makefile* out) const {
  LocalWriteMakeInternal(true, out);
}

void JavaJarNode::LocalWriteMakeInternal(bool write_base, Makefile* out) const {
  JavaLibraryNode::LocalWriteMakeInternal(false, out);
  WriteRules(out);
  ResourceFileSet files;
  files.Add(JarName());
  WriteBaseUserTarget(files, out);
}

void JavaJarNode::WriteRules(Makefile* out) const {
  // Collect object directores.
  ResourceFileSet roots;
  ObjectRoots(JAVA, &roots);

  // Move all objects in those directories to our JarRoot.
  ResourceFileSet temp_files;
  for (const Resource& input : roots.files()) {
    temp_files.Add(MoveFiles(JarRoot(), input, out));
  }

  // Now run jar on the root.
  WriteJar(JarName(), JarRoot(), temp_files, out);
}

Resource JavaJarNode::MoveFiles(const Resource& root,
                                const Resource& input,
                                Makefile* out) const {
  Resource touchfile = Touchfile(strings::Base16Encode(input.path()));
  Makefile::Rule* rule = out->StartRule(touchfile.path(), input.path());
  string relative_path = strings::GetRelativePath(root.path(), input.dirname());
  rule->WriteCommand("mkdir -p " + root.path());
  string file = strings::JoinPath(relative_path, "$file");
  rule->WriteCommand(
      Makefile::Escape(
          "FILES=$(cd " + input.dirname() + "; find . -type f -o -type l); "
          "cd " + root.path() + "; "
          "for file in $FILES; do"
          " mkdir -p $(dirname $file);"
          " RELATIVE=$(FILE=$(dirname $file); while [ \"$FILE\" != \".\" ]; "
          "  do printf '../'; FILE=$(dirname $FILE); done); "
          " ln -s -f $RELATIVE" + file + " $file; "
          "done"));
  rule->WriteCommand("mkdir -p " + touchfile.dirname());
  rule->WriteCommand("touch " + touchfile.path());
  out->FinishRule(rule);
  return touchfile;
}

void JavaJarNode::WriteJar(const Resource& jar_file,
                           const Resource& root,
                           const ResourceFileSet& dependencies,
                           Makefile* out) const {
  Resource manifest = WriteManifest(out);

  // Collect flags.
  set<string> flags;
  LinkFlags(JAVA, &flags);
  for (const string& f : input().flags("-J")) {
    flags.insert(f);
  }

  // Jar rule
  Makefile::Rule* rule = out->StartRule(
      jar_file.path(),
      strings::JoinWith(" ",
                        manifest.path(),
                        strings::JoinAll(dependencies.files(), " ")));
  rule->WriteUserEcho("Jaring", jar_file.path());
  rule->WriteCommand("mkdir -p " + root.path());
  rule->WriteCommand(strings::JoinWith(
      " ",
      "cd " + root.path(),
      "; jar cfm",
      strings::GetRelativePath(root.path(), jar_file.path()),
      strings::GetRelativePath(root.path(), manifest.path()),
      strings::JoinAll(flags, " "),
      "$$(find . -type f -o -type l)"));
  out->FinishRule(rule);
}

Resource JavaJarNode::WriteManifest(Makefile* out) const {
  Resource manifest = Resource::FromLocalPath(
      strings::JoinPath(input().genfile_dir(),
                        target().dir()),
      target().local_path() + ".manifest");
  string man_cmd = "eval 'rm -f " + manifest.path() + "; for line in";
  for (const string& str : java_manifest_) {
    man_cmd += " \"" + str + "\"";
  }
  man_cmd += "; do echo \"$$line\" >> " + manifest.path() + "; done; ";
  man_cmd += "touch " + manifest.path() + "'";
  Makefile::Rule* rule = out->StartRule(manifest.path());
  rule->WriteCommand("mkdir -p " + manifest.dirname());
  rule->WriteCommand(man_cmd);
  out->FinishRule(rule);
  return manifest;
}

Resource JavaJarNode::JarName() const {
  return Resource::FromLocalPath(input().object_dir(),
                                 target().make_path() + ".jar");
}

Resource JavaJarNode::JarRoot() const {
  return Resource::FromLocalPath(input().pkgfile_dir(),
                                 target().make_path());
}

}  // namespace repobuild
