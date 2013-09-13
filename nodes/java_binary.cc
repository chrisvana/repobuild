// Copyright 2013
// Author: Christopher Van Arsdale

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
#include "repobuild/reader/buildfile.h"

using std::string;
using std::vector;
using std::set;

namespace repobuild {

void JavaBinaryNode::Parse(BuildFile* file, const BuildFileNode& input) {
  JavaLibraryNode::Parse(file, input);
  
  current_reader()->ParseRepeatedString("java_manifest", &java_manifest_);

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
}

void JavaBinaryNode::LocalWriteMake(Makefile* out) const {
  JavaLibraryNode::LocalWriteMakeInternal(false, out);

  // Output binary
  Resource bin = JarName();
  WriteJar(bin, out);

  {  // Output user target
    ResourceFileSet deps;
    deps.Add(bin);
    WriteBaseUserTarget(deps, out);
  }

  // Symlink to root dir.
  Resource out_bin = OutJarName();
  out->StartRule(out_bin.path(), bin.path());
  out->WriteCommand("pwd > /dev/null");  // hack to work around make issue?
  out->WriteCommand("ln -f -s " + bin.path() + " " + out_bin.path());
  out->FinishRule();
}

void JavaBinaryNode::WriteJar(const Resource& file, Makefile* out) const {
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
  out->StartRule(manifest.path(), strings::JoinAll(objects.files(), " "));
  out->WriteCommand(man_cmd);
  out->FinishRule();

  // Jar rule
  out->StartRule(file.path(),
                 strings::JoinWith(" ",
                                   manifest.path(),
                                   strings::JoinAll(objects.files(), " ")));
  out->WriteCommand("echo Jaring: " + file.path());
  out->WriteCommand(strings::JoinWith(
      " ",
      "cd " + input().object_dir(),
      "; jar cfm",
      "../" + file.path(),
      "../" + manifest.path(),
      strings::JoinAll(flags, " "),
      strings::JoinAll(class_paths, " ")));
  out->FinishRule();
}

void JavaBinaryNode::LocalWriteMakeClean(Makefile* out) const {
  out->WriteCommand("rm -f " + OutJarName().path());
}

void JavaBinaryNode::LocalFinalOutputs(LanguageType lang,
                                       ResourceFileSet* outputs) const {
  JavaLibraryNode::LocalFinalOutputs(lang, outputs);
  outputs->Add(OutJarName());
}

void JavaBinaryNode::LocalBinaries(LanguageType lang,
                                   ResourceFileSet* outputs) const {
  outputs->Add(JarName());
}

Resource JavaBinaryNode::JarName() const {
  return Resource::FromLocalPath(input().object_dir(),
                                 target().make_path() + ".jar");
}

Resource JavaBinaryNode::OutJarName() const {
  return Resource::FromLocalPath(input().root_dir(),
                                 target().local_path() + ".jar");
}

}  // namespace repobuild
