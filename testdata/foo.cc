// Copyright 2013
// Author: Christopher Van Arsdale

#include "foo.h"
#include "a/foo.h"
#include "b/foo.h"
#include "c/foo.h"
#include "d/foo.h"

namespace test {

void RunFooTest() {
  a::RunFooA();
  b::RunFooB();
  c::RunFooC();
  d::RunFooD();
}

}  // namespace test
