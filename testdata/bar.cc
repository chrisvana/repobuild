// Copyright 2013
// Author: Christopher Van Arsdale

#include <iostream>
#include "bar.h"
#include "testdata/bar_data.txt.h"

namespace b {
extern void RunBarB();
}

namespace test {

void RunBarTest() {
  b::RunBarB();
  std::cout << "Embed size: " << embed_bar_data_txt_size() << std::endl
            << "Embed data: " << embed_bar_data_txt_data() << std::endl;
}

}  // namespace test
