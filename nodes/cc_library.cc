// Copyright 2013
// Author: Christopher Van Arsdale

#include <string>
#include <set>
#include <iterator>
#include <vector>
#include "common/log/log.h"
#include "common/file/fileutil.h"
#include "common/strings/path.h"
#include "common/strings/strutil.h"
#include "repobuild/env/input.h"
#include "nodes/cc_library.h"
#include "repobuild/reader/buildfile.h"

using std::vector;
using std::string;
using std::set;

namespace repobuild {

void CCLibraryNode::Parse(BuildFile* file, const BuildFileNode& input) {
  Node::Parse(file, input);

  // cc_sources
  ParseRepeatedFiles(input, "cc_sources", &sources_);

  // cc_headers
  ParseRepeatedFiles(input, "cc_headers", &headers_);
  
  // cc_objs
  ParseRepeatedFiles(input, "cc_objects", &objects_);

  // cc_compile_args
  ParseRepeatedString(input, "cc_compile_args", &cc_compile_args_);

  // cc_linker_args
  ParseRepeatedString(input, "cc_linker_args", &cc_linker_args_);
}

void CCLibraryNode::Set(const vector<string>& sources,
                        const vector<string>& headers,
                        const vector<string>& objects,
                        const vector<string>& cc_compile_args) {
  sources_ = sources;
  headers_ = headers;
  objects_ = objects;
  cc_compile_args_ = cc_compile_args;
}

void CCLibraryNode::WriteMakefile(const vector<const Node*>& all_deps,
                                  string* out) const {
  // Figure out the set of input files.
  set<string> input_files;
  CollectDependencies(all_deps, &input_files);

  // Now write phases, one per .cc
  for (int i = 0; i < sources_.size(); ++i) {
    // Output object.
    WriteCompile(sources_[i], input_files, out);
  }
}

void CCLibraryNode::WriteCompile(const string& source,
                                 const set<string>& input_files,
                                 string* out) const {
  string obj = strings::JoinPath(input().object_dir(), source + ".o");
  out->append(obj + ":");

  // Dependencies.
  for (const string& input : input_files) {
    out->append(" ");
    out->append(input);
  }
  out->append(" ");
  out->append(source);

  // Mkdir command.
  out->append("\n\t");
  out->append("mkdir -p ");
  out->append(strings::PathDirname(obj));

    // Compile command.
  out->append("; ");
  out->append(DefaultCompileFlags());
  out->append(" -c");
  out->append(" -I");
  out->append(input().root_dir());
  out->append(" -I");
  out->append(input().genfile_dir());
  out->append(" -I");
  out->append(input().source_dir());
  for (const string& flag : input().flags("-C")) {
    out->append(" ");
    out->append(flag);
  }
  for (int j = 0; j < cc_compile_args_.size(); ++j) {
    out->append(" ");
    out->append(cc_compile_args_[j]);
  }
  out->append(" ");
  out->append(source);

  // Output object.
  out->append(" -o ");
  out->append(obj);

  out->append("\n\n");
}

void CCLibraryNode::DependencyFiles(vector<string>* files) const {
  Node::DependencyFiles(files);
  for (int i = 0; i < headers_.size(); ++i) {
    files->push_back(headers_[i]);
  }
}

void CCLibraryNode::ObjectFiles(vector<string>* files) const {
  Node::ObjectFiles(files);
  for (int i = 0; i < sources_.size(); ++i) {
    files->push_back(strings::JoinPath(input().object_dir(),
                                       sources_[i] + ".o"));
  }
  for (const string& obj : objects_) {
    files->push_back(strings::JoinPath(target().dir(), obj));
  }
}

void CCLibraryNode::LinkFlags(std::set<std::string>* flags) const {
  Node::LinkFlags(flags);
  for (const string flag : cc_linker_args_) {
    flags->insert(flag);
  }
}

std::string CCLibraryNode::DefaultCompileFlags() const {
  return "$(CXX) $(CXXFLAGS)";
}

}  // namespace repobuild
