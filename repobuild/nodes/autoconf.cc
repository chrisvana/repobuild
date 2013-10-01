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
namespace {  // TODO(cvanarsdale): Shared location with cc_library.cc.
const char kConfigureArgs[] = "CONFIGURE_ARGS";
const char kConfigureEnv[] = "CONFIGURE_ENV";
const char kCGcc[] = "CC_GCC";
const char kCxxGcc[] = "CXX_GCC";
}

void AutoconfNode::Parse(BuildFile* file, const BuildFileNode& input) {
  Node::Parse(file, input);

  // configure_env
  vector<string> configure_env, gcc_configure_env, clang_configure_env;
  current_reader()->ParseRepeatedString("configure_env", &configure_env);
  current_reader()->ParseRepeatedString("gcc.configure_env",
                                        &gcc_configure_env);
  current_reader()->ParseRepeatedString("clang.configure_env",
                                        &clang_configure_env);

  // configure_args
  vector<string> configure_args, gcc_configure_args, clang_configure_args;
  current_reader()->ParseRepeatedString("configure_args", &configure_args);
  current_reader()->ParseRepeatedString("gcc.configure_args",
                                        &gcc_configure_args);
  current_reader()->ParseRepeatedString("clang.configure_args",
                                        &clang_configure_args);

  // configure
  string configure;
  current_reader()->ParseStringField("configure_cmd", &configure);
  if (configure.empty()) {
    configure = "./configure";
  }

  // Generate the output files.
  GenShNode* gen = new GenShNode(
      target().GetParallelTarget(file->NextName(target().local_path())),
      Node::input());
  for (const TargetInfo& dep : dep_targets()) {
    gen->AddDependencyTarget(dep);
  }
  gen->SetMakeName("Autoconf");
  AddSubNode(gen);

  // Env
  AddConditionalVariable(
      kConfigureEnv, kCxxGcc,
      strings::JoinWith(
          " ",
          strings::JoinAll(configure_env, " "),
          strings::JoinAll(gcc_configure_env, " ")),
      strings::JoinWith(
          " ",
          strings::JoinAll(configure_env, " "),
          strings::JoinAll(clang_configure_env, " ")));

  // Args
  AddConditionalVariable(
      kConfigureArgs, kCxxGcc,
      strings::JoinWith(
          " ",
          strings::JoinAll(configure_args, " "),
          strings::JoinAll(gcc_configure_args, " ")),
      strings::JoinWith(
          " ",
          strings::JoinAll(configure_args, " "),
          strings::JoinAll(clang_configure_args, " ")));

  // Actual configure command output ------
  string build_setup = Makefile::Escape(
      "mkdir -p $OBJ_DIR; "
      "DEST_DIR=$(pwd)/$GEN_DIR");
  string build_env = GetVariable(kConfigureEnv).ref_name() +
      Makefile::Escape(
          " CXXFLAGS=\"$BASIC_CXXFLAGS $DEP_FLAGS $USER_CXXFLAGS\" "
          "CFLAGS=\"$BASIC_CFLAGS $DEP_FLAGS $USER_CFLAGS\" "
          "LDFLAGS=\"$LDFLAGS $USER_LDFLAGS\" "
          "CC=\"$CC\" "
          "CXX=\"$CXX\"");
  string configure_cmd =
      Makefile::Escape(configure +
                       " --prefix=/ --cache-file=$GEN_DIR/config.cache ") +
      GetVariable(kConfigureArgs).ref_name();
  vector<Resource> input_files;
  vector<string> output_files;
  gen->Set(build_setup + "; " + build_env + " " + configure_cmd,
           "",  // clean
           input_files,
           output_files);
  gen->SetMakefileEscape(false);  // we do it ourselves

  // Make output --------------------------
  MakeNode* make = new MakeNode(
      target().GetParallelTarget(file->NextName(target().local_path())),
      Node::input());
  AddSubNode(make);
  make->AddDependencyTarget(gen->target());
  make->Parse(file, input);
}

}  // namespace repobuild
