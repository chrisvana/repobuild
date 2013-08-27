// Copyright 2013
// Author: Christopher Van Arsdale

#include <string>
#include <vector>
#include "common/base/flags.h"
#include "common/strings/path.h"
#include "repobuild/env/input.h"
#include "repobuild/env/target.h"

DEFINE_bool(add_default_flags, true,
            "If false, we disable the default flags.");

// TODO(cvanarsdale): A default configuration file ('.repobuild') that contains
// flags. We can search the path/tree/homedir for it.

using std::string;

namespace repobuild {

Input::Input() {
  current_path_ = strings::CurrentPath();
  root_dir_ = ".";
  full_root_dir_ = strings::JoinPath(current_path_, root_dir_);
  object_dir_ = ".gen-obj";
  full_object_dir_ = strings::JoinPath(current_path_, object_dir_);
  genfile_dir_ = ".gen-files";
  full_genfile_dir_ = strings::JoinPath(current_path_, genfile_dir_);
  source_dir_ = ".gen-src";

  // Default flags.
  if (FLAGS_add_default_flags) {
    // Compiling
    AddFlag("-X", "-std=c++11");
    AddFlag("-X", "-DUSE_CXX0X");
    AddFlag("-C", "clang=-stdlib=libc++");  // clang only, see cc_library.cc
    AddFlag("-C", "-pthread");
    AddFlag("-C", "-g");
    AddFlag("-C", "-Wall");
    AddFlag("-C", "-Werror");
    AddFlag("-C", "-Wno-sign-compare");
    AddFlag("-C", "gcc=-Wno-unused-local-typedefs");
    AddFlag("-C", "gcc=-Wno-error=unused-local-typedefs");
    AddFlag("-C", "-O3");
    AddFlag("-C", "-flto");
    AddFlag("-C", "clang=-Qunused-arguments");  // clang only, see cc_library.cc

    // Linking
    AddFlag("-L", "clang=-stdlib=libc++");
    AddFlag("-L", "-lpthread");
    AddFlag("-L", "-g");
    AddFlag("-L", "-O3");
    AddFlag("-L", "-flto");
    AddFlag("-L", "-L/usr/local/lib");
    AddFlag("-L", "-L/opt/local/lib");
  }
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
