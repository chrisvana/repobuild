// Copyright 2013
// Author: Christopher Van Arsdale

#include "testdata/foo.h"
#include "testdata/a/foo.h"
#include "testdata/b/foo.h"
#include "c/foo.h"
#include "testdata/d/foo.h"

namespace test {

void RunFooTest() {
  a::RunFooA();
  b::RunFooB();
  c::RunFooC();
  d::RunFooD();
}

}  // namespace test
