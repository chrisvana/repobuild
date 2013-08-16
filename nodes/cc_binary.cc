// Copyright 2013
// Author: Christopher Van Arsdale

#include <set>
#include <string>
#include <vector>
#include <iterator>
#include "common/log/log.h"
#include "common/strings/path.h"
#include "repobuild/env/input.h"
#include "nodes/cc_binary.h"
#include "repobuild/reader/buildfile.h"

using std::string;
using std::vector;
using std::set;

namespace repobuild {

void CCBinaryNode::Parse(BuildFile* file, const BuildFileNode& input) {
  CCLibraryNode::Parse(file, input);
}

void CCBinaryNode::WriteMakefile(const vector<const Node*>& all_deps,
                                 string* out) const {
  CCLibraryNode::WriteMakefileInternal(all_deps, false, out);

  // Output binary
  string bin = strings::JoinPath(input().object_dir(), target().make_path());
  WriteLink(all_deps, bin, out);

  // Output user target
  out->append(target().make_path());
  out->append(": ");
  out->append(bin);
  out->append("\n\n.PHONY: ");
  out->append(target().make_path());
  out->append("\n\n");

  // Symlink to root dir.
  string out_bin = OutBinary();
  out->append(out_bin);
  out->append(": ");
  out->append(bin);
  out->append("\n\t");
  out->append("pwd > /dev/null");  // hack to work around make issue?
  out->append("\n\tln -f -s ");
  out->append(strings::JoinPath(input().object_dir(), target().make_path()));
  out->append(" ");
  out->append(out_bin);
  out->append("\n\n");
}

void CCBinaryNode::WriteLink(
    const vector<const Node*>& all_deps,
    const string& file,
    string* out) const {
  set<string> objects;
  CollectObjects(all_deps, &objects);

  set<string> flags;
  CollectLinkFlags(all_deps, &flags);

  string list;
  for (const string& obj : objects) {
    list.append(" ");
    list.append(obj);
  }

  out->append(file + ":");
  out->append(list);
  out->append("\n\t");
  out->append("$(LINK.cc)");
  out->append(list);
  out->append(" -o ");
  out->append(file);
  for (const string& flag : flags) {
    out->append(" ");
    out->append(flag);
  }
  out->append("\n\n");
}

void CCBinaryNode::WriteMakeClean(std::string* out) const {
  out->append("\trm -f ");
  out->append(OutBinary());
  out->append("\n");
}

void CCBinaryNode::FinalOutputs(vector<string>* outputs) const {
  outputs->push_back(OutBinary());
}

std::string CCBinaryNode::OutBinary() const {
  return strings::JoinPath(input().root_dir(), target().local_path());
}

}  // namespace repobuild
