// Copyright 2013
// Author: Christopher Van Arsdale

#include "foo.h"
#include "a/foo.h"
#include "b/foo.h"

namespace test {

void RunFooTest() {
  a::RunFooA();
  b::RunFooB();
}

}  // namespace test
