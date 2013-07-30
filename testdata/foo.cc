// Copyright 2013
// Author: Christopher Van Arsdale

#include "foo.h"
#include "a/foo.h"
#include "b/foo.h"
#include "c/foo.h"

namespace test {

void RunFooTest() {
  a::RunFooA();
  b::RunFooB();
  c::RunFooC();
}

}  // namespace test
