// Copyright 2013
// Author: Christopher Van Arsdale

#include <map>
#include <string>
#include <set>
#include <iterator>
#include <vector>
#include "common/log/log.h"
#include "repobuild/env/input.h"
#include "repobuild/nodes/cmake.h"
#include "repobuild/nodes/gen_sh.h"
#include "repobuild/nodes/make.h"
#include "repobuild/reader/buildfile.h"

using std::map;
using std::vector;
using std::string;

namespace repobuild {

void CmakeNode::Parse(BuildFile* file, const BuildFileNode& input) {
  Node::Parse(file, input);

  // configure_env
  vector<string> cmake_envs;
  ParseRepeatedString(input, "cmake_env", &cmake_envs);

  // cmake_args
  vector<string> cmake_args;
  ParseRepeatedString(input, "cmake_args", &cmake_args);

  // Generate the output files.
  GenShNode* gen = new GenShNode(target().GetParallelTarget(file->NextName()),
                                 Node::input());
  for (const TargetInfo* dep : dependencies()) {
    gen->AddDependency(*dep);
  }
  AddSubNode(gen);

  // Users are allowed to specify custom env arg overrides.
  string user_env;
  if (!cmake_envs.empty()) {
    for (const string& it : cmake_envs) {
      user_env.append(" " + it);
    }
    user_env.append("; ");
  }

  // Actual cmake  command output ------
  string build_setup =
      "BASE=$(pwd); "
      "DEST_DIR=$(pwd)/$GEN_DIR; "
      "mkdir -p $DEST_DIR/build; "
      "STAGING=$DEST_DIR/.staging; "
      "cd $GEN_DIR/build";
  string build_env = user_env +"CC=$CC CXX=$CXX ";
  string cmake_cmd =
      "cmake -DCMAKE_INSTALL_PREFIX=. -B $BASE "
      "-DCMAKE_CXX_FLAGS=\"$BASIC_CXXFLAGS $USER_CXXFLAGS\" "
      "-DCMAKE_C_FLAGS=\"$BASIC_CFLAGS $USER_CFLAGS\"";
  for (const string& it : cmake_args) {
    cmake_cmd.append(" " + it);
  }
  vector<string> input_files, output_files;
  gen->Set(build_setup + "; " + build_env + " " + cmake_cmd,
           "",  // clean
           input_files,
           output_files);

  // Make output --------------------------
  string postinstall_cmd =
      "(if [ -d \"$STAGING/$BASE\" ]; then"
      " (for f in $(ls -d $STAGING/$RDBASE/*); do"
      "  rm $DEST_DIR/$(basename $f); mv $f $DEST_DIR || exit 1;"
      " done) &&"
      " rm -rf $STAGING && find . -type d -empty -delete; else echo -n ''; "
      "fi)";
  MakeNode* make = new MakeNode(target().GetParallelTarget(file->NextName()),
                                Node::input());
  AddSubNode(make);
  make->AddDependency(gen->target());
  make->ParseWithDirAndPostinstall(file, input, "$STAGING", postinstall_cmd);
}

}  // namespace repobuild
