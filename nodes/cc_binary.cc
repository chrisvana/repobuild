// Copyright 2013
// Author: Christopher Van Arsdale

#include <set>
#include <string>
#include <vector>
#include <iterator>
#include "common/log/log.h"
#include "common/strings/path.h"
#include "env/input.h"
#include "nodes/cc_binary.h"
#include "reader/buildfile.h"

using std::string;
using std::vector;
using std::set;

namespace repobuild {

void CCBinaryNode::Parse(BuildFile* file, const BuildFileNode& input) {
  CCLibraryNode::Parse(file, input);
  ParseRepeatedString(input, "cc_linker_args", &cc_linker_args_);
}

void CCBinaryNode::WriteMakefile(const Input& input,
                                 const vector<const Node*>& all_deps,
                                 string* out) const {
  CCLibraryNode::WriteMakefile(input, all_deps, out);

  // Object files.
  set<string> object_files;
  for (int i = 0; i < all_deps.size(); ++i) {
    vector<string> obj_files;
    all_deps[i]->ObjectFiles(input, &obj_files);
    for (const string& it : obj_files) { object_files.insert(it); }
  }
  {
    vector<string> obj_files;
    ObjectFiles(input, &obj_files);
    for (const string& it : obj_files) { object_files.insert(it); }
  }

  // Output binary
  string bin = strings::JoinPath(input.object_dir(),
                                 strings::JoinPath(target().dir(),
                                                   target().local_path()));
  out->append(bin + ":");
  for (const string& input : object_files) {
    out->append(" ");
    out->append(input);
  }
  out->append("\n\t");
  out->append("mkdir -p obj/" + target().dir());
  out->append(
      "; clang++ -std=c++11 -stdlib=libc++ -pthread -DUSE_CXX0X -g ");
  for (int i = 0; i < cc_compile_args_.size(); ++i) {
    out->append(" ");
    out->append(cc_compile_args_[i]);
  }
  for (const string& input : object_files) {
    out->append(" ");
    out->append(input);
  }
  out->append(" -o ");
  out->append(bin);
  out->append("\n\n");
}

}  // namespace repobuild
