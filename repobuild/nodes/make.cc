// Copyright 2013
// Author: Christopher Van Arsdale

#include <map>
#include <string>
#include <set>
#include <iterator>
#include <vector>
#include "common/log/log.h"
#include "common/strings/path.h"
#include "repobuild/env/input.h"
#include "repobuild/env/resource.h"
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
  current_reader()->ParseStringField("postinstall", true, &user_postinstall);

  // make_target
  string make_target;
  if (!current_reader()->ParseStringField("make_target", true, &make_target)) {
    make_target = "install";
  }

  // make_file
  string make_file;
  vector<Resource> files;
  current_reader()->ParseSingleFile("make_file", &files);
  if (files.size() == 0) {
    make_file = "Makefile";
  } else if (files.size() == 1) {
    make_file = strings::GetRelativePath(target().dir(), files[0].path());
  } else {
    LOG(FATAL) << "make_file must be single file: "
               << target().full_path();
  }

  // Generate the output files.
  GenShNode* gen = NewSubNodeWithCurrentDeps<GenShNode>(file);
  gen->SetCd(true);
  gen->SetMakeName("Make");

  string make_cmd = ("$MAKE DESTDIR=" + dest_dir + " -f " + make_file + " "
                     + make_target);
  if (!preinstall.empty()) {
    make_cmd = preinstall + " && " + make_cmd;
  }
  if (!postinstall.empty()) {
    make_cmd += " && " + postinstall;
  }
  if (!user_postinstall.empty()) {
    make_cmd += " && " + user_postinstall;
  }
  string clean_cmd = ("$MAKE DESTDIR=" + dest_dir + " clean > /dev/null 2>&1 "
                      "|| echo -n \"\"");  // always succeed.

  vector<Resource> input_files;
  current_reader()->ParseRepeatedFiles("inputs", &input_files);

  vector<Resource> output_files;
  current_reader()->ParseRepeatedFiles("outs", false, &output_files);

  gen->Set(make_cmd,
           clean_cmd,
           input_files,
           output_files);
}

}  // namespace repobuild
