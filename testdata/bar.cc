// Copyright 2013
// Author: Christopher Van Arsdale

#include <iostream>
#include "bar.h"
#include "testdata/bar_data.h"

namespace b {
extern void RunBarB();
}

namespace test {

void RunBarTest() {
  b::RunBarB();
  std::cout << "Embed size: "
            << ::testdata::bar_data::embed_bar_data_txt_size() << std::endl
            << "Embed data: "
            << ::testdata::bar_data::embed_bar_data_txt_data() << std::endl
            << "Embed expected size: "
            << strlen(::testdata::bar_data::embed_bar_data_txt_data())
            << std::endl;

  std::cout << "Embed 2 size: "
            << ::testdata::bar_data::embed_bar_data2_txt_size() << std::endl
            << "Embed 2 data: "
            << ::testdata::bar_data::embed_bar_data2_txt_data() << std::endl
            << "Embed expected size: "
            << strlen(::testdata::bar_data::embed_bar_data2_txt_data())
            << std::endl;
}

}  // namespace test
