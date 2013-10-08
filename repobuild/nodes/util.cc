// Copyright 2013
// Author: Christopher Van Arsdale

#include <string>
#include "common/log/log.h"
#include "common/strings/path.h"
#include "common/strings/strutil.h"
#include "repobuild/env/input.h"
#include "repobuild/env/target.h"
#include "repobuild/nodes/util.h"

using std::string;

namespace repobuild {

// static
string NodeUtil::StripSpecialDirs(const Input& input, const string& path) {
  string dir = path;
  while (true) {
    if (strings::HasPrefix(dir, input.genfile_dir())) {
      dir = dir.substr(std::min(input.genfile_dir().size() + 1, dir.size()));
      continue;
    }
    if (strings::HasPrefix(dir, input.source_dir())) {
      dir = dir.substr(std::min(input.source_dir().size() + 1, dir.size()));
      continue;
    }
    if (strings::HasPrefix(dir, input.object_dir())) {
      dir = dir.substr(std::min(input.object_dir().size() + 1, dir.size()));
      continue;
    }
    if (strings::HasPrefix(dir, input.pkgfile_dir())) {
      dir = dir.substr(std::min(input.pkgfile_dir().size() + 1, dir.size()));
      continue;
    }
    break;
  }
  return dir;
}

ComponentHelper::ComponentHelper(const std::string& component,
                                 const std::string& base_dir)
    : component_(component),
      base_dir_(base_dir) {
}

string ComponentHelper::RewriteFile(const Input& input,
                                    const string& path) const {
  string file = NodeUtil::StripSpecialDirs(input, path);
  if (!base_dir_.empty() && strings::HasPrefix(file, base_dir_ + "/")) {
    file = file.substr(base_dir_.size() + 1);
    if (!component_.empty()) {
      return strings::JoinPath(component_, file);
    }
  }
  return file;
}

bool ComponentHelper::RewriteDependency(TargetInfo* target) const {
  if (target->top_component() == component_) {
    string new_dir;
    if (target->dir().size() > component_.size()) {
      new_dir = strings::JoinPath(base_dir_,
                                  target->dir().substr(component_.size() + 1));
    } else {
      new_dir = base_dir_;
    }
    *target = TargetInfo("//" + new_dir + ":" + target->local_path());
    return true;
  }
  return false;
}

bool ComponentHelper::CoversPath(const Input& input, const string& path) const {
  string file = NodeUtil::StripSpecialDirs(input, path);
  return strings::HasPrefix(file, base_dir_);
}

}  // namespace repobuild
