// Copyright 2013
// Author: Christopher Van Arsdale

#include <set>
#include <string>
#include <vector>
#include "common/log/log.h"
#include "common/strings/path.h"
#include "repobuild/env/input.h"
#include "nodes/gen_sh.h"
#include "repobuild/reader/buildfile.h"

#include "json/json.h"

using std::string;
using std::vector;
using std::set;

namespace repobuild {

void GenShNode::Parse(BuildFile* file, const BuildFileNode& input) {
  Node::Parse(file, input);
  if (!ParseStringField(input, "build_cmd", &build_cmd_)) {
    LOG(FATAL) << "Could not parse build_cmd.";
  }
  ParseStringField(input, "clean", &clean_cmd_);
  ParseRepeatedFiles(input, "input_files", &input_files_);
  ParseRepeatedString(input, "outs", &outputs_);
}

void GenShNode::WriteMakefile(const vector<const Node*>& all_deps,
                              string* out) const {
  // Figure out the set of input files.
  set<string> input_files;
  for (int i = 0; i < all_deps.size(); ++i) {
    vector<string> files;
    all_deps[i]->DependencyFiles(&files);
    for (const string& it : files) {
      input_files.insert(it);
    }
  }
  for (const string& it : input_files_) {
    input_files.insert(it);
  }

  out->append(target().make_path());
  out->append(":");
  for (const string& it : input_files) {
    out->append(" ");
    out->append(it);
  }
  out->append("\n");

  if (!build_cmd_.empty()) {
    out->append("\t");
    out->append(WriteCommand(clean_cmd_));
    out->append("\n");
  }
  out->append("\n");

  for (const string& output : outputs_) {
    out->append(strings::JoinPath(GenDir(), output));
    out->append(": ");
    out->append(target().make_path());
    out->append("\n\n");
  }

  out->append(".PHONY: ");
  out->append(target().make_path());
  out->append("\n\n");
}

void GenShNode::WriteMakeClean(std::string* out) const {
  if (clean_cmd_.empty()) {
    return;
  }

  out->append("\t");
  out->append(WriteCommand(clean_cmd_));
  out->append("\n");
}

string GenShNode::WriteCommand(const string& cmd) const {
  string out;
  out.append("(mkdir -p ");
  out.append(GenDir());
  out.append("; cd ");
  out.append(target().dir());
  out.append("; env GEN_DIR=\"");
  out.append(RelativeGenDir());
  out.append("\" ");
  out.append(build_cmd_);
  out.append(")");
  return out;
}

}  // namespace repobuild
