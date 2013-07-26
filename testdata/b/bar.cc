// Copyright 2013
// Author: Christopher Van Arsdale

#include <iostream>

namespace a {
extern void RunBarA();
}

namespace b {

void RunBarB() {
  a::RunBarA();
  std::cout << "BarB" << std::endl;
}

}  // namespace b
