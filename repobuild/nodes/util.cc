// Copyright 2013
// Author: Christopher Van Arsdale

#include <string>
#include "common/strings/strutil.h"
#include "repobuild/env/input.h"
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

}  // namespace repobuild
