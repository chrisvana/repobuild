// Copyright 2013
// Author: Christopher Van Arsdale

#include <iostream>
#include "common/base/init.h"
#include "common/log/log.h"
#include "env/input.h"
#include "generator/generator.h"

int main(int argc, const char** argv) {
  InitProgram();
  repobuild::Input input(".",  "obj");
  for (int i = 1; i < argc; ++i) {
    input.AddBuildTarget(argv[i]);
  }
  repobuild::Generator generator;
  std::cout << generator.GenerateMakefile(input) << std::endl;
}
