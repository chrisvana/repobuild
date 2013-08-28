// Copyright 2013
// Author: Christopher Van Arsdale

#include <map>
#include <string>
#include <set>
#include <iterator>
#include <vector>
#include "common/log/log.h"
#include "repobuild/env/input.h"
#include "repobuild/nodes/autoconf.h"
#include "repobuild/nodes/gen_sh.h"
#include "repobuild/nodes/make.h"
#include "repobuild/reader/buildfile.h"

using std::map;
using std::vector;
using std::string;

namespace repobuild {

void AutoconfNode::Parse(BuildFile* file, const BuildFileNode& input) {
  Node::Parse(file, input);

  // configure_env
  vector<string> configure_envs;
  ParseRepeatedString(input, "configure_env", &configure_envs);

  // configure_args
  vector<string> configure_args;
  ParseRepeatedString(input, "configure_args", &configure_args);

  // Generate the output files.
  GenShNode* gen = new GenShNode(target().GetParallelTarget(file->NextName()),
                                 Node::input());
  for (const TargetInfo* dep : dependencies()) {
    gen->AddDependency(*dep);
  }
  AddSubNode(gen);

  // Users are allowed to specify custom env arg overrides.
  string user_env;
  if (!configure_envs.empty()) {
    for (const string& it : configure_envs) {
      user_env.append(" " + it);
    }
    user_env.append("; ");
  }

  // Actual configure command output ------
  string build_setup =
      "mkdir -p $OBJ_DIR; "
      "DEST_DIR=$(pwd)/$GEN_DIR";
  string build_env =
      user_env +
      "CXXFLAGS=\"$BASIC_CXXFLAGS $DEP_FLAGS $USER_CXXFLAGS\" "
      "CFLAGS=\"$BASIC_CFLAGS $DEP_FLAGS $USER_CFLAGS\" "
      "LDFLAGS=\"$LDFLAGS $USER_LDFLAGS\" "
      "CC=\"$CC\" "
      "CXX=\"$CXX\"";
  string configure_cmd =
      "./configure --prefix=/ --cache-file=$GEN_DIR/config.cache";
  for (const string& it : configure_args) {
    configure_cmd.append(" " + it);
  }
  vector<string> input_files, output_files;
  gen->Set(build_setup + "; " + build_env + " " + configure_cmd,
           "",  // clean
           input_files,
           output_files);

  // Make output --------------------------
  MakeNode* make = new MakeNode(target().GetParallelTarget(file->NextName()),
                                Node::input());
  AddSubNode(make);
  make->AddDependency(gen->target());
  make->Parse(file, input);
}

}  // namespace repobuild
