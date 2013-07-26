// Copyright 2013
// Author: Christopher Van Arsdale

#include "env/input.h"
#include "generator/parser.h"

int main(int argc, const char** argv) {
  repobuild::Input input("",  "obj_tmp");
  for (int i = 1; i < argc; ++i) {
    input.AddBuildTarget(argv[i]);
  }
  repobuild::Parser parser;
  parser.Parse(input);
}
