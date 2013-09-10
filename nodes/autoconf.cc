// Copyright 2013
// Author: Christopher Van Arsdale

#include <map>
#include <string>
#include <set>
#include <iterator>
#include <vector>
#include "common/log/log.h"
#include "common/strings/path.h"
#include "common/strings/strutil.h"
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
  current_reader()->ParseRepeatedString("configure_env", &configure_envs);

  // configure_args
  vector<string> configure_args;
  current_reader()->ParseRepeatedString("configure_args", &configure_args);

  string configure_dir;
  current_reader()->ParseStringField("configure_dir", &configure_dir);
  string configure = (configure_dir.empty() ? "./configure" :
                      strings::JoinPath(configure_dir, "configure"));

  // Generate the output files.
  GenShNode* gen = new GenShNode(
      target().GetParallelTarget(file->NextName(target().local_path())),
      Node::input());
  for (const TargetInfo* dep : dependencies()) {
    gen->AddDependency(*dep);
  }
  AddSubNode(gen);

  // Users are allowed to specify custom env arg overrides.
  string user_env;
  if (!configure_envs.empty()) {
    user_env = strings::JoinAll(configure_envs, " ") + "; ";
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
  string configure_cmd = (configure +
                          " --prefix=/ --cache-file=$GEN_DIR/config.cache " +
                          strings::JoinAll(configure_args, " "));
  vector<Resource> input_files;
  vector<string> output_files;
  gen->Set(build_setup + "; " + build_env + " " + configure_cmd,
           "",  // clean
           input_files,
           output_files);

  // Make output --------------------------
  MakeNode* make = new MakeNode(
      target().GetParallelTarget(file->NextName(target().local_path())),
      Node::input());
  AddSubNode(make);
  make->AddDependency(gen->target());
  make->Parse(file, input);
}

}  // namespace repobuild
