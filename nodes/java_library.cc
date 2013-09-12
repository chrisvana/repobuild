// Copyright 2013
// Author: Christopher Van Arsdale

#include <set>
#include <string>
#include <vector>
#include "common/log/log.h"
#include "common/strings/path.h"
#include "common/strings/strutil.h"
#include "repobuild/env/input.h"
#include "repobuild/nodes/java_library.h"
#include "repobuild/reader/buildfile.h"

using std::vector;
using std::string;
using std::set;

namespace repobuild {

void JavaLibraryNode::Parse(BuildFile* file, const BuildFileNode& input) {
  Node::Parse(file, input);

  // java_sources
  current_reader()->ParseRepeatedFiles("java_sources", &sources_);

  // root dir for output class files, which is also a class path down below,
  // see Init().
  current_reader()->ParseStringField("java_out_root", &java_out_root_);

  // classpath info.
  vector<Resource> java_classpath_dirs;
  current_reader()->ParseRepeatedFiles("java_additional_classpaths",
                                       false,  // directory need not exist.
                                       &java_classpath_dirs);
  for (const Resource& r : java_classpath_dirs) {
    java_classpath_.push_back(r.path());
  }

  // javac args
  current_reader()->ParseRepeatedString("java_local_compile_args",
                                        &java_local_compile_args_);
  current_reader()->ParseRepeatedString("java_compile_args",  // inherited.
                                        &java_compile_args_);

  // jar args
  current_reader()->ParseRepeatedString("java_jar_args",
                                        &java_jar_args_);

  Init();
}

void JavaLibraryNode::Set(const vector<Resource>& sources) {
  sources_ = sources;
  Init();
}

void JavaLibraryNode::Init() {
  for (const Resource& source : sources_) {
    CHECK(strings::HasSuffix(source.path(), ".java"))
        << "Invalid java source "
        << source << " in target " << target().full_path();
  }
  if (java_out_root_.empty()) {
    java_out_root_ = input().object_dir();
  } else {
    java_out_root_ = strings::JoinPath(ObjectDir(), java_out_root_);
  }
  java_classpath_.push_back(java_out_root_); 
}

void JavaLibraryNode::LocalWriteMakeInternal(bool write_user_target,
                                             Makefile* out) const {
  // Figure out the set of input files.
  ResourceFileSet input_files;
  DependencyFiles(&input_files);

  // Compile all .java files at the same time, for efficiency.
  WriteCompile(input_files, out);

  // Now write user target (so users can type "make path/to/exec|lib").
  if (write_user_target) {
    ResourceFileSet targets;
    for (const Resource& source : sources_) {
      targets.Add(ClassFile(source));
    }
    WriteBaseUserTarget(targets, out);
  }
}

void JavaLibraryNode::WriteCompile(const ResourceFileSet& input_files,
                                   Makefile* out) const {
  set<string> directories;
  string obj_files;
  for (const Resource& source : sources_) {
    Resource obj = ClassFile(source);
    obj_files += (!obj_files.empty() ? " " : "") + obj.path();
    directories.insert(obj.dirname());
  }

  // Rule=> obj1 obj2 obj3: <input header files> source1.java source.java ...
  out->StartRule(obj_files,
                 strings::JoinWith(" ",
                                   strings::JoinAll(input_files.files(), " "),
                                   strings::JoinAll(sources_, " ")));

  // Mkdir commands.
  for (const string d : directories) {
    out->WriteCommand("mkdir -p " + d);
  }

  // Compile command.
  string compile = "javac";

  // Collect class paths.
  set<string> java_classpath;
  IncludeDirs(&java_classpath);

  // class path.
  string include_dirs = strings::JoinWith(
      " ",
      "-cp ",
      strings::JoinWith(":",
                        input().root_dir(),
                        input().genfile_dir(),
                        input().source_dir(),
                        strings::JoinPath(input().source_dir(),
                                          input().genfile_dir()),
                        strings::JoinAll(java_classpath, ":")));

  // javac compile args.
  set<string> compile_args;
  CompileFlags(false /* ignored */, &compile_args);
  compile_args.insert(java_local_compile_args_.begin(),
                      java_local_compile_args_.end());
  for (const string& f : input().flags("-JC")) {
    compile_args.insert(f);
  }

  out->WriteCommand("echo Compiling: " + target().make_path());
  out->WriteCommand(strings::JoinWith(
      " ",
      compile,
      "-d " + java_out_root_,
      "-s " + input().genfile_dir(),
      strings::JoinAll(compile_args, " "),
      include_dirs,
      strings::JoinAll(sources_, " ")));

  out->FinishRule();
}

void JavaLibraryNode::LocalLinkFlags(std::set<std::string>* flags) const {
  flags->insert(java_jar_args_.begin(), java_jar_args_.end());
}

void JavaLibraryNode::LocalCompileFlags(bool cxx,
                                   std::set<std::string>* flags) const {
  flags->insert(java_compile_args_.begin(), java_compile_args_.end());
}

void JavaLibraryNode::LocalIncludeDirs(std::set<std::string>* dirs) const {
  dirs->insert(java_classpath_.begin(), java_classpath_.end());
}

void JavaLibraryNode::LocalObjectFiles(ResourceFileSet* files) const {
  Node::LocalObjectFiles(files);
  for (const Resource& r : sources_) {
    files->Add(ClassFile(r));
  }
}

Resource JavaLibraryNode::ClassFile(const Resource& source) const {
  CHECK(strings::HasSuffix(source.path(), ".java"));
  return Resource::FromRootPath(
      strings::JoinPath(
          input().object_dir(),
          source.path().substr(0, source.path().size() - 4) + "class"));
}

}  // namespace repobuild
