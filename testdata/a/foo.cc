// Copyright 2013
// Author: Christopher Van Arsdale

#include <iostream>
#include "testdata/a/foo.h"
#include "testdata/a/a.pb.h"

namespace a {

void RunFooA() {
  test::FooProto proto;
  std::cout << "FooA" << proto.DebugString() << std::endl;
}

}  // namespace a
