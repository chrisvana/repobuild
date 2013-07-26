// Copyright 2013
// Author: Christopher Van Arsdale

#include <string>
#include <vector>
#include "env/input.h"

namespace repobuild {

const std::vector<std::string>& Input::flags(const std::string& key) const {
  auto it = flags_.find(key);
  if (it == flags_.end()) {
    static std::vector<std::string> kEmpty;
    return kEmpty;
  }
  return it->second;
}

}  // namespace repobuild
