// Copyright 2013
// Author: Christopher Van Arsdale

#include <string>
#include <ostream>
#include "common/strings/path.h"
#include "common/strings/strutil.h"
#include "repobuild/env/resource.h"

using std::string;

namespace repobuild {

// static
Resource Resource::FromRootPath(const string& path) {
  Resource out;
  out.root_path_ = path;
  out.basename_ = strings::PathBasename(path);
  out.dirname_ = strings::PathDirname(path);
  return out;
}

// static
Resource Resource::FromLocalPath(const std::string& path,
                                 const std::string& local) {
  return FromRootPath(strings::JoinPath(path, local));
}

// static
Resource Resource::FromRaw(const std::string& raw) {
  Resource out;
  out.root_path_ = raw;
  return out;
}

std::ostream& operator<<(std::ostream& o, const Resource& r) {
  return o << r.path();
}

}  // namespace repobuild
