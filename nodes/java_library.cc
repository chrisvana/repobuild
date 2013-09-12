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

  ParseInternal(file, input);
}

void JavaLibraryNode::Set(BuildFile* file,
                          const BuildFileNode& input,
                          const vector<Resource>& sources) {
  Node::Parse(file, input);
  sources_ = sources;
  ParseInternal(file, input);
}

void JavaLibraryNode::ParseInternal(BuildFile* file,
                                    const BuildFileNode& input) {
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

  // Sanity checks.
  for (const Resource& source : sources_) {
    CHECK(strings::HasSuffix(source.path(), ".java"))
        << "Invalid java source "
        << source << " in target " << target().full_path();
  }
  if (java_out_root_.empty()) {
    java_out_root_ = Node::input().object_dir();
  } else {
    java_out_root_ = strings::JoinPath(ObjectDir(), java_out_root_);
  }
  java_classpath_.push_back(java_out_root_); 
}

void JavaLibraryNode::LocalWriteMakeInternal(bool write_user_target,
                                             Makefile* out) const {
  // Figure out the set of input files.
  ResourceFileSet input_files;
  InputDependencyFiles(JAVA, &input_files);

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
  vector<Resource> obj_files;
  set<string> directories;
  for (const Resource& source : sources_) {
    obj_files.push_back(ClassFile(source));
    directories.insert(obj_files.back().dirname());
  }

  // Rule=> obj1 obj2 obj3: <input header files> source1.java source.java ...
  out->StartRule(strings::JoinAll(obj_files, " " ),
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
  IncludeDirs(JAVA, &java_classpath);

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
  CompileFlags(JAVA, &compile_args);
  compile_args.insert(java_local_compile_args_.begin(),
                      java_local_compile_args_.end());
  for (const string& f : input().flags("-JC")) {
    compile_args.insert(f);
  }

  out->WriteCommand("echo \"Compiling: " + target().make_path() + " (java)\"");
  out->WriteCommand(strings::JoinWith(
      " ",
      compile,
      "-d " + java_out_root_,
      "-s " + input().genfile_dir(),
      strings::JoinAll(compile_args, " "),
      include_dirs,
      strings::JoinAll(sources_, " ")));

  // Make sure we actually generated all of the object files, otherwise the
  // user may have specified the wrong java_out_root.
  for (const Resource& obj : obj_files) {
    out->WriteCommand("if [ ! -f " + obj.path() + " ]; then " +
                      "echo \"Class file not generated: " + obj.path() +
                      ", or it was generated in an unexpected location. Make "
                      "sure java_out_root is specified correctly or the "
                      "package name for the object is: " +
                      strings::Replace(obj.dirname(), "/", ".") +
                      "\"; exit 1; fi");
  }

  out->FinishRule();
}

void JavaLibraryNode::LocalLinkFlags(LanguageType lang,
                                     std::set<std::string>* flags) const {
  if (lang == JAVA) {
    flags->insert(java_jar_args_.begin(), java_jar_args_.end());
  }
}

void JavaLibraryNode::LocalCompileFlags(LanguageType lang,
                                        std::set<std::string>* flags) const {
  if (lang == JAVA) {
    flags->insert(java_compile_args_.begin(), java_compile_args_.end());
  }
}

void JavaLibraryNode::LocalIncludeDirs(LanguageType lang,
                                       std::set<std::string>* dirs) const {
  dirs->insert(java_classpath_.begin(), java_classpath_.end());
}

void JavaLibraryNode::LocalObjectFiles(LanguageType lang,
                                       ResourceFileSet* files) const {
  Node::LocalObjectFiles(lang, files);
  for (const Resource& r : sources_) {
    files->Add(ClassFile(r));
  }
}

void JavaLibraryNode::LocalDependencyFiles(LanguageType lang,
                                           ResourceFileSet* files) const {
  for (const Resource& r : sources_) {
    files->Add(r);
  }

  // Java also needs class files for dependent javac invocations.
  LocalObjectFiles(lang, files);
}

Resource JavaLibraryNode::ClassFile(const Resource& source) const {
  CHECK(strings::HasSuffix(source.path(), ".java"));
  return Resource::FromRootPath(
      strings::JoinPath(
          input().object_dir(),
          StripSpecialDirs(source.path().substr(0, source.path().size() - 4)) +
          "class"));
}

}  // namespace repobuild
