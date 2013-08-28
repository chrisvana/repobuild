// Copyright 2013
// Author: Christopher Van Arsdale

#include <map>
#include <string>
#include <set>
#include <iterator>
#include <vector>
#include "common/log/log.h"
#include "repobuild/env/input.h"
#include "repobuild/nodes/make.h"
#include "repobuild/nodes/gen_sh.h"
#include "repobuild/reader/buildfile.h"

using std::map;
using std::vector;
using std::string;

namespace repobuild {

void MakeNode::ParseWithOptions(BuildFile* file,
                                const BuildFileNode& input,
                                const string& preinstall,
                                const string& dest_dir,
                                const string& postinstall) {
  Node::Parse(file, input);

  // configure_args
  string user_postinstall;
  ParseStringField(input, "postinstall", &user_postinstall);

  // Generate the output files.
  GenShNode* gen = new GenShNode(target().GetParallelTarget(file->NextName()),
                                 Node::input());
  for (const TargetInfo* dep : dependencies()) {
    gen->AddDependency(*dep);
  }
  AddSubNode(gen);

  string make_cmd = "$MAKE install DESTDIR=" + dest_dir;
  if (!preinstall.empty()) {
    make_cmd = preinstall + " && " + make_cmd;
  }
  if (!postinstall.empty()) {
    make_cmd += " && " + postinstall;
  }
  if (!user_postinstall.empty()) {
    make_cmd += " && " + user_postinstall;
  }
  string clean_cmd = "$MAKE DESTDIR=" + dest_dir + " clean";

  vector<string> input_files;
  ParseRepeatedFiles(input, "inputs", &input_files);

  vector<string> output_files;
  ParseRepeatedString(input, "outs", &output_files);

  gen->Set(make_cmd,
           clean_cmd,
           input_files,
           output_files);
}

}  // namespace repobuild
