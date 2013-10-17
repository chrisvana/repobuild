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

DEFINE_bool(enable_flto_object_files, true,
            "If true, we enable -flto in the default flags.");

DEFINE_bool(silent_make, true,
            "If false, make prints out commands before execution.");

DEFINE_string(proto_cc_library, "//third_party/protobuf:cc_proto",
              "Default cc_library for proto_library to include.");

DEFINE_string(proto_java_library, "//third_party/protobuf:java_proto",
              "Default java_library for proto_library to include.");

DEFINE_string(proto_go_library, "//third_party/protobuf:go_proto",
              "Default go_library for proto_library to include.");

DEFINE_string(proto_py_library, "//third_party/protobuf:py_proto",
              "Default cc_library for proto_library to include.");

// TODO(cvanarsdale): A default configuration file ('.repobuild') that contains
// flags. We can search the path/tree/homedir for it.

DEFINE_string(root_dir, ".",
              "Location of our source root.");

DEFINE_string(object_dir, ".gen-obj",
              "Location where we store compiled objects.");

DEFINE_string(genfile_dir, ".gen-files",
              "Location where we store generated files.");

DEFINE_string(source_dir, ".gen-src",
              "Location where we do horrible linker munging to make "
              "component files look like they are on the right path.");

DEFINE_string(package_dir, ".gen-pkg",
              "Location where we put packaged files (e.g. symlinked into a "
              "specific directory structure). This could be the merged version "
              "of .gen-src and .gen-files");

DEFINE_string(binary_dir, "bin",
              "Location where we put final output binaries (in addition to "
              "root dir).");

DEFINE_bool(debug, false,
            "If true, we disable optimizations.");

using std::string;

namespace repobuild {

Input::Input() {
  root_dir_ = FLAGS_root_dir;
  full_root_dir_ = strings::JoinPath(strings::CurrentPath(), root_dir_);
  object_dir_ = FLAGS_object_dir;
  genfile_dir_ = FLAGS_genfile_dir;
  source_dir_ = FLAGS_source_dir;
  pkgfile_dir_ = FLAGS_package_dir;
  binary_dir_ = FLAGS_binary_dir;

  // Default flags.
  if (FLAGS_add_default_flags) {
    // Compiling
    AddFlag("-X", "-std=c++11");
    AddFlag("-X", "-DUSE_CXX0X");
    AddFlag("-C", "clang=-stdlib=libc++");
    AddFlag("-C", "-pthread");
    AddFlag("-C", "-g");
    AddFlag("-C", "-Wall");
    AddFlag("-C", "-Werror");
    AddFlag("-C", "-Wno-sign-compare");
    AddFlag("-C", "gcc=-Wno-unused-local-typedefs");
    AddFlag("-C", "gcc=-Wno-error=unused-local-typedefs");
    if (!FLAGS_debug) {
      AddFlag("-C", "-O3");
      if (FLAGS_enable_flto_object_files) {
        AddFlag("-C", "-flto");
      }
    }
    AddFlag("-C", "clang=-Qunused-arguments");
    AddFlag("-C", "clang=-fcolor-diagnostics");

    // Linking
    AddFlag("-L", "clang=-stdlib=libc++");
    AddFlag("-L", "-lpthread");
    AddFlag("-L", "-g");
    if (!FLAGS_debug) {
      AddFlag("-L", "-O3");
      if (FLAGS_enable_flto_object_files) {
        AddFlag("-L", "-flto");
      }
    }
    AddFlag("-L", "-L/usr/local/lib");
    AddFlag("-L", "-L/opt/local/lib");

    // Java compiler
    AddFlag("-JC", "-g");
  }

  silent_make_ = FLAGS_silent_make;

  default_cc_proto_ = FLAGS_proto_cc_library;
  default_java_proto_ = FLAGS_proto_java_library;
  default_go_proto_ = FLAGS_proto_go_library;
  default_py_proto_ = FLAGS_proto_py_library;
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
