// Copyright 2013
// Author: Christopher Van Arsdale

#include "foo.h"

namespace test {
extern void RunBarTest();
}

int main(int argc, char** argv) {
  test::RunBarTest();
  test::RunFooTest();
  return 0;
}
