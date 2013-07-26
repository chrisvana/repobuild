// Copyright 2013
// Author: Christopher Van Arsdale

#include "bar.h"

namespace b {
extern void RunBarB();
}

namespace test {

void RunBarTest() {
  b::RunBarB();
}

}  // namespace test
