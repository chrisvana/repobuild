// Copyright 2013
// Author: Christopher Van Arsdale

#include <string>
#include <vector>
#include "common/base/flags.h"
#include "common/file/fileutil.h"
#include "common/log/log.h"
#include "common/strings/path.h"
#include "common/strings/strutil.h"
#include "common/util/shell.h"
#include "repobuild/env/input.h"
#include "repobuild/env/target.h"
#include "repobuild/reader/buildfile.h"
#include "repobuild/third_party/json/json.h"

DEFINE_bool(add_default_flags, true,
            "If false, we disable the default flags.");

DEFINE_bool(enable_flto_object_files, true,
            "If true, we enable -flto in the default flags.");

DEFINE_bool(silent_make, true,
            "If false, make prints out commands before execution.");

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
}

const std::vector<std::string>& Input::flags(const std::string& key) const {
  auto it = flags_.find(key);
  if (it == flags_.end()) {
    static std::vector<std::string> kEmpty;
    return kEmpty;
  }
  return it->second;
}

static void GetAllBuildSubdirs(const std::string& dir,
			       std::vector<std::string>* out) {
  std::string output;
  util::Execute("/usr/bin/find " + dir + " -maxdepth 2 -name BUILD", &output);
  std::vector<std::string> subdirs = strings::SplitString(output, "\n");
  for (std::string subdir : subdirs) {
    CHECK(strings::HasSuffix(subdir, "/BUILD")) << subdir;
    std::string no_build = subdir.substr(0, subdir.length() - 6);
    if (no_build != ".") {
      out->push_back(no_build);
    }
  }
}

void Input::AddBuildTarget(const TargetInfo& target) {
  if (build_target_set_.insert(target.full_path()).second) {
    if (target.IsAll()) {
      BuildFile* file = new BuildFile(target.build_file());
      // Parse the BUILD into a structured format.
      string filestr = file::ReadFileToStringOrDie(file->filename());
      file->Parse(filestr);
      for (BuildFileNode* node : file->nodes()) {
	LOG_IF(FATAL, !node->object().isObject())
          << "Expected json object (file = " << file->filename() << "): "
          << node->object();
	for (const string& key : node->object().getMemberNames()) {
	  if (key == "config" || key == "plugin") {
	    continue;
	  }
	  const Json::Value& value = node->object()[key];
	  const Json::Value& name = value["name"];
	  LOG_IF(FATAL, name.asString() == "all") <<
	    "Invalid node named \"all\" in " << target.build_file();
	  if (name.isString()) {
	    string path = target.dir() + ":" + name.asString();
	    AddBuildTarget(TargetInfo::FromUserPath(path));
	  }
	}
      }
      if (target.IsRec()) {
	std::vector<std::string> subdirs;
	GetAllBuildSubdirs(target.dir(), &subdirs);
	for (std::string subdir : subdirs) {
          AddBuildTarget(TargetInfo::FromUserPath(subdir + ":allrec"));
	}
      }
    } else {
      LG << "processing " << target.full_path();
      build_targets_.push_back(target);
    }
  }
}

}  // namespace repobuild
