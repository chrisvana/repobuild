// Copyright 2013
// Author: Christopher Van Arsdale

#include <string>
#include <set>
#include <iterator>
#include <vector>
#include "common/log/log.h"
#include "repobuild/env/input.h"
#include "repobuild/nodes/autoconf.h"
#include "repobuild/nodes/gen_sh.h"
#include "repobuild/reader/buildfile.h"

using std::vector;
using std::string;

namespace repobuild {

void AutoconfNode::Parse(BuildFile* file, const BuildFileNode& input) {
  Node::Parse(file, input);

  // Generate the output files.
  GenShNode* gen = new GenShNode(target().GetParallelTarget(file->NextName()),
                                 Node::input());
  for (const TargetInfo* dep : dependencies()) {
    gen->AddDependency(*dep);
  }
  AddSubNode(gen);

  string build_setup =
      "mkdir -p $OBJ_DIR; "
      "DEST_DIR=$(pwd)/$GEN_DIR";
  string build_env =
      "CXXFLAGS=\"$BASIC_CXXFLAGS $DEP_FLAGS\" "
      "CFLAGS=\"$BASIC_CFLAGS $DEP_FLAGS\" "
      "LDFLAGS=\"$LDFLAGS\" "
      "CC=\"$CC\" "
      "CXX=\"$CXX\"";

  // TODO(cvanarsdale): Configurable flags.
  string configure_cmd =
      "./configure --prefix=/ --cache-file=$GEN_DIR/config.cache";

  string make_cmd =
      "make install DESTDIR=$(pwd)/$GEN_DIR";

  string clean_cmd =
      "make DESTDIR=$(pwd)/$GEN_DIR clean";

  vector<string> input_files;
  ParseRepeatedFiles(input, "inputs", &input_files);

  SetStrictFileMode(false);
  vector<string> output_files;
  ParseRepeatedString(input, "outs", &output_files);

  gen->Set(build_setup + "; " + build_env + " " + configure_cmd + " && " +
           make_cmd,
           clean_cmd,
           input_files,
           output_files);
}

}  // namespace repobuild
