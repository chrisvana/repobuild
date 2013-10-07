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
  WriteJar(JarName(), out);
  WriteBaseUserTarget(out);
}

void JavaJarNode::WriteJar(const Resource& file, Makefile* out) const {
  // Collect objects, and strip .obj-dir prefix.
  ResourceFileSet objects;
  vector<string> class_paths;
  ObjectFiles(JAVA, &objects);
  for (const Resource& r : objects.files()) {
    // "jar" runs from the object directory.
    if (!strings::HasPrefix(r.path(), input().object_dir() + "/")) {
      LOG(FATAL) << "Jar needs all objects to be in object_dir: "
                 << input().object_dir()
                 << "\", but have \"" << r << "\"";
    }
    class_paths.push_back(r.path().substr(input().object_dir().size() + 1));
  }

  // Collect flags.
  set<string> flags;
  LinkFlags(JAVA, &flags);
  for (const string& f : input().flags("-J")) {
    flags.insert(f);
  }

  // Write manifest file.
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
  Makefile::Rule* rule = out->StartRule(manifest.path(),
                                        strings::JoinAll(objects.files(), " "));
  rule->WriteCommand(man_cmd);
  out->FinishRule(rule);

  // Jar rule
  rule = out->StartRule(file.path(),
                 strings::JoinWith(" ",
                                   manifest.path(),
                                   strings::JoinAll(objects.files(), " ")));
  rule->WriteUserEcho("Jaring", file.path());
  rule->WriteCommand(strings::JoinWith(
      " ",
      "cd " + input().object_dir(),
      "; jar cfm",
      "../" + file.path(),
      "../" + manifest.path(),
      strings::JoinAll(flags, " "),
      strings::JoinAll(class_paths, " ")));
  out->FinishRule(rule);
}

Resource JavaJarNode::JarName() const {
  return Resource::FromLocalPath(input().object_dir(),
                                 target().make_path() + ".jar");
}

}  // namespace repobuild
