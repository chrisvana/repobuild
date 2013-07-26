// Copyright 2013
// Author: Christopher Van Arsdale

#include <string>
#include <vector>
#include "common/log/log.h"
#include "common/strings/path.h"
#include "common/strings/strutil.h"
#include "common/strings/stringpiece.h"
#include "env/target.h"

namespace repobuild {
namespace {
void CheckPath(const std::string& path) {
  CHECK(strings::HasPrefix(path, "//"));
  std::vector<StringPiece> pieces = strings::Split(path, ":");
  CHECK_EQ(2, pieces.size());
}

std::string BuildFileFromTarget(const std::string& target) {
  // already validated.
  std::vector<StringPiece> pieces =
      strings::Split(StringPiece(target).substr(2), ":");
  return strings::JoinPath(pieces[0], "BUILD");
}
}

TargetInfo::TargetInfo(const std::string& full_path)
    : full_path_(full_path) {
  CheckPath(full_path_);
  full_path_ = "/" + strings::CleanPath(full_path_.substr(1));
  CheckPath(full_path_);
  build_file_ = BuildFileFromTarget(full_path_);
}

TargetInfo::TargetInfo(const std::string& relative_path,
                       const std::string& build_file)
    : build_file_(strings::CleanPath(build_file)) {
  CHECK(strings::HasSuffix(build_file_, "/BUILD") || build_file_ == "BUILD");
  full_path_ = "//" +
      strings::JoinPath(build_file.substr(0, build_file_.size() - 5),
                        relative_path);
  CheckPath(full_path_);
}

}  // namespace repobuild
