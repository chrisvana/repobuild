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
  for (const Resource& source : sources_) {
    CHECK(strings::HasSuffix(source.path(), ".java"))
        << "Invalid java source "
        << source << " in target " << target().full_path();
  }

  // javac args
  current_reader()->ParseRepeatedString("java_local_compile_args",
                                        &java_local_compile_args_);
  current_reader()->ParseRepeatedString("java_compile_args",  // inherited.
                                        &java_compile_args_);

  // jar args
  current_reader()->ParseRepeatedString("java_jar_args",
                                        &java_jar_args_);
}

void JavaLibraryNode::WriteMakefileInternal(bool write_user_target,
                                            Makefile* out) const {
  // Figure out the set of input files.
  set<Resource> input_files;
  DependencyFiles(&input_files);

  // Now write phases, one per .cc
  for (int i = 0; i < sources_.size(); ++i) {
    // Output object.
    WriteCompile(sources_[i], input_files, out);
  }

  // Now write user target (so users can type "make path/to/exec|lib").
  if (write_user_target) {
    set<Resource> targets;
    for (const Resource& source : sources_) {
      targets.insert(ClassFile(source));
    }
    WriteBaseUserTarget(targets, out);
  }
}

void JavaLibraryNode::WriteCompile(const Resource& source,
                                   const set<Resource>& input_files,
                                   Makefile* out) const {
  Resource obj = ClassFile(source);

  // Rule=> obj: <input header files> source.cc
  out->StartRule(obj.path(),
                 strings::JoinWith(" ",
                                   strings::JoinAll(input_files, " "),
                                   source.path()));

  // Mkdir command.
  out->WriteCommand("mkdir -p " + obj.dirname());

  // Compile command.
  string compile = "javac";

  // class path.
  string include_dirs = strings::JoinWith(
      " ",
      "-cp ",
      strings::JoinWith(":",
                        input().root_dir(),
                        input().genfile_dir(),
                        input().source_dir(),
                        strings::JoinPath(input().source_dir(),
                                          input().genfile_dir())));

  // javac compile args.
  set<string> compile_args;
  CompileFlags(false /* ignored */, &compile_args);
  compile_args.insert(java_local_compile_args_.begin(),
                      java_local_compile_args_.end());
  for (const string& f : input().flags("-JC")) {
    compile_args.insert(f);
  }

  out->WriteCommand("echo Compiling: " + source.path());
  out->WriteCommand(strings::JoinWith(
      " ",
      compile,
      "-d " + input().object_dir(),
      "-s " + input().genfile_dir(),
      strings::JoinAll(compile_args, " "),
      include_dirs,
      source.path()));

  out->FinishRule();
}

void JavaLibraryNode::LinkFlags(std::set<std::string>* flags) const {
  flags->insert(java_jar_args_.begin(), java_jar_args_.end());
}

void JavaLibraryNode::CompileFlags(bool cxx,
                                   std::set<std::string>* flags) const {
  flags->insert(java_compile_args_.begin(), java_compile_args_.end());
}

void JavaLibraryNode::ObjectFiles(ObjectFileSet* files) const {
  Node::ObjectFiles(files);
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
