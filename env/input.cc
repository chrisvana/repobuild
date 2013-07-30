// Copyright 2013
// Author: Christopher Van Arsdale

#include <string>
#include <vector>
#include "repobuild/env/input.h"
#include "repobuild/env/target.h"
#include "common/strings/path.h"

using std::string;

namespace repobuild {

Input::Input() {
  current_path_ = strings::CurrentPath();
  root_dir_ = ".";
  full_root_dir_ = strings::JoinPath(current_path_, root_dir_);
  object_dir_ = "obj";
  full_object_dir_ = strings::JoinPath(current_path_, object_dir_);
  source_dir_ = "src";

  // Default flags.
  // Compiling
  AddFlag("-C", "-std=c++11");
  AddFlag("-C", "-DUSE_CXX0X");
  AddFlag("-C", "-stdlib=libc++");
  AddFlag("-C", "-pthread");
  AddFlag("-C", "-g");
  AddFlag("-C", "-Wall");
  AddFlag("-C", "-O3");
  // Linking
  AddFlag("-L", "-std=c++11");
  AddFlag("-L", "-DUSE_CXX0X");
  AddFlag("-L", "-stdlib=libc++");
  AddFlag("-L", "-lpthread");
  AddFlag("-L", "-g");
  AddFlag("-L", "-O3");
}

const std::vector<std::string>& Input::flags(const std::string& key) const {
  auto it = flags_.find(key);
  if (it == flags_.end()) {
    static std::vector<std::string> kEmpty;
    return kEmpty;
  }
  return it->second;
}

void Input::AddBuildTarget(const TargetInfo& target) {
  if (build_target_set_.insert(target.full_path()).second) {
    build_targets_.push_back(target);
  }
}

}  // namespace repobuild
