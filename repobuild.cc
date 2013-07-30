// Copyright 2013
// Author: Christopher Van Arsdale

#include <iostream>
#include "common/base/init.h"
#include "common/log/log.h"
#include "common/file/fileutil.h"
#include "common/strings/path.h"
#include "common/strings/strutil.h"
#include "common/strings/stringpiece.h"
#include "repobuild/env/input.h"
#include "repobuild/env/target.h"
#include "repobuild/generator/generator.h"

int main(int argc, const char** argv) {
  InitProgram();
  repobuild::Input input;
  for (int i = 1; i < argc; ++i) {
    if (strings::HasPrefix(argv[i], "-")) {
      int pos = StringPiece(argv[i]).find('=');
      if (pos == StringPiece::npos) {
        LOG(FATAL) << "Invalid repobuild flag, require -flag=value.";
      }
      input.AddFlag(StringPiece(argv[i]).substr(0, pos).as_string(),
                    StringPiece(argv[i]).substr(pos+1).as_string());
    } else {
      input.AddBuildTarget(
          repobuild::TargetInfo::FromUserPath(argv[i]).full_path());
    }
  }
  repobuild::Generator generator;
  file::WriteFileOrDie(strings::JoinPath(input.root_dir(), "Makefile"),
                       generator.GenerateMakefile(input));
  return 0;
}
