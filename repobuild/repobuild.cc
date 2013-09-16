// Copyright 2013
// Author: Christopher Van Arsdale
//
// Repobuild usage:
//  ./repobuild [flag]* [targets]+
// [flag] => see env/input.cc
//           Format is -FLAG_TYPE=FLAG_VALUE, e.g. -X=-Wno-error=asdf
//           Compiler conditional args look like: -X=gcc=... or -X=clang=...
// [targets] => see env/target.cc
//              format is "path/to:target" or "//path/to:target"
//
// To build repobuild...
// 1) With a make file:
//  make repobuild
// 2) With repobuild itself:
//  ./repbuild ":repobuild" && make repobuild
//

#include <iostream>
#include "common/base/init.h"
#include "common/base/flags.h"
#include "common/log/log.h"
#include "common/file/fileutil.h"
#include "common/strings/path.h"
#include "common/strings/strutil.h"
#include "common/strings/stringpiece.h"
#include "repobuild/env/input.h"
#include "repobuild/env/target.h"
#include "repobuild/generator/generator.h"

DEFINE_string(makefile, "Makefile",
              "Name of makefile output.");

namespace {
const char* kUsage =
    "\n\n"
    "  To generate makefile:\n"
    "     repobuild \"path/to/dir:target\" [--makefile=Makefile]\n"
    "\n"
    "  To build:\n"
    "     make [-j8] [target]\n"
    "\n"
    "  To run:\n"
    "     ./.gen-obj/path/to/target\n"
    "         or\n"
    "     ./target";
}

int main(int argc, char** argv) {
  InitProgram(&argc, &argv, kUsage, true);
  repobuild::Input input;

  // Parse command line arguments.
  // 1) Arguments for compilation (-C=a, -X=a, -L=a, etc ... see env/input.cc)
  // 2) Build targets (e.g. ":repobuild" "common/strings/testing:strutil_test")
  for (int i = 1 /* 0 == binary name */; i < argc; ++i) {
    // (1) Command line args.
    if (strings::HasPrefix(argv[i], "-")) {
      // Format is -<T>=<flag>, like -X=-Wno-error=unused-local-typedefs
      int pos = StringPiece(argv[i]).find('=');
      if (pos == StringPiece::npos) {
        LOG(FATAL) << "Invalid repobuild flag, require -flag=value.";
      }
      input.AddFlag(StringPiece(argv[i]).substr(0, pos).as_string(),
                    StringPiece(argv[i]).substr(pos+1).as_string());
    } else {
      // (2) Anything not a flag better be a build target.
      input.AddBuildTarget(repobuild::TargetInfo::FromUserPath(argv[i]));
    }
  }

  // Generate the output Makefile.
  repobuild::Generator generator;
  file::WriteFileOrDie(strings::JoinPath(input.root_dir(), FLAGS_makefile),
                       generator.GenerateMakefile(input));
  return 0;
}
