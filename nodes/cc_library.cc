// Copyright 2013
// Author: Christopher Van Arsdale

#include <string>
#include <set>
#include <iterator>
#include <vector>
#include "common/log/log.h"
#include "common/strings/path.h"
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
  ParseRepeatedString(input, "cc_sources", &sources_);
  for (int i = 0; i < sources_.size(); ++i) {
    sources_[i] = strings::JoinPath(target().dir(), sources_[i]);
  }

  // cc_headers
  ParseRepeatedString(input, "cc_headers", &headers_);
  for (int i = 0; i < headers_.size(); ++i) {
    headers_[i] = strings::JoinPath(target().dir(), headers_[i]);
  }

  // cc_compile_args
  ParseRepeatedString(input, "cc_compile_args", &cc_compile_args_);
}

void CCLibraryNode::WriteMakefile(const Input& input,
                                  const vector<const Node*>& all_deps,
                                  string* out) const {
  // Figure out the set of input files.
  set<string> input_files;
  for (int i = 0; i < all_deps.size(); ++i) {
    vector<string> files;
    all_deps[i]->DependencyFiles(input, &files);
    for (const string& it : files) { input_files.insert(it); }
  }
  {
    vector<string> files;
    DependencyFiles(input, &files);
    for (const string& it : files) { input_files.insert(it); }
    for (const string& it : headers_) { input_files.insert(it); }
  }

  // Now write phases, one per .cc
  for (int i = 0; i < sources_.size(); ++i) {
    // Output object.
    string obj = strings::JoinPath(input.object_dir(), sources_[i] + ".o");
    out->append(obj + ":");

    // Dependencies.
    for (const string& input : input_files) {
      out->append(" ");
      out->append(input);
    }
    out->append(" ");
    out->append(sources_[i]);

    // Mkdir command.
    out->append("\n\t");
    out->append("mkdir -p ");
    out->append(strings::PathDirname(obj));

    // Compile command.
    out->append(
        "; clang++ -std=c++11 -stdlib=libc++ -pthread -DUSE_CXX0X -g -c");
    out->append(" -I");
    out->append(input.root_dir());
    out->append(" -I");
    out->append(input.source_dir());
    for (int j = 0; j < cc_compile_args_.size(); ++j) {
      out->append(" ");
      out->append(cc_compile_args_[j]);
    }
    out->append(" ");
    out->append(sources_[i]);

    // Output object.
    out->append(" -o ");
    out->append(obj);

    out->append("\n\n");
  }
}

void CCLibraryNode::DependencyFiles(const Input& input,
                                    vector<string>* files) const {
  Node::DependencyFiles(input, files);
  for (int i = 0; i < headers_.size(); ++i) {
    files->push_back(headers_[i]);
  }
}

void CCLibraryNode::ObjectFiles(const Input& input,
                                vector<string>* files) const {
  Node::ObjectFiles(input, files);
  for (int i = 0; i < sources_.size(); ++i) {
    files->push_back(strings::JoinPath(input.object_dir(),
                                       sources_[i] + ".o"));
  }
}

}  // namespace repobuild
