// Copyright 2013
// Author: Christopher Van Arsdale

#include <string>
#include <vector>
#include "common/log/log.h"
#include "common/strings/path.h"
#include "common/strings/strutil.h"
#include "common/strings/stringpiece.h"
#include "repobuild/env/target.h"

using std::string;
using std::vector;

namespace repobuild {
namespace {
bool IsValidPath(const string& path) {
  if (!strings::HasPrefix(path, "//")) {
    return false;
  }
  vector<StringPiece> pieces = strings::Split(path, ":");
  return pieces.size() == 2;
}

void CheckPath(const string& path) {
  if (!IsValidPath(path)) {
    LOG(FATAL) << "Invalid path: " << path;
  }
}

string BuildDir(const string& target) {
  // already validated.
  vector<StringPiece> pieces =
      strings::SplitAllowEmpty(StringPiece(target).substr(2), ":");
  CHECK_EQ(2, pieces.size())
      << "Could not parse target: " << target;
  return pieces[0].as_string();
}

string CleanFullPath(const string& path) {
  CheckPath(path);
  string str = "/" + strings::CleanPath(path.substr(1));

  // Fix //a/:b -> //a:b
  size_t pos = str.rfind('/');
  if (pos > 1 && str.size() > pos + 1 && str[pos + 1] == ':') {
    str = str.substr(0, pos) + str.substr(pos + 1);
  }

  return str;
}

string LocalPath(const string& path) {
  // already validated.
  vector<StringPiece> pieces =
      strings::SplitAllowEmpty(StringPiece(path).substr(2), ":");
  CHECK_EQ(2, pieces.size()) << "Could not parse path: " << path;
  return pieces[1].as_string();
}

string TopComponent(const string& dir) {
  size_t pos = dir.find('/');
  if (pos == string::npos) {
    return dir;
  }
  return dir.substr(0, pos);
}

}  // anonymous namespace

TargetInfo::TargetInfo(const string& full_path)
    : full_path_(full_path),
      was_relative_(false) {
  CheckPath(full_path_);
  full_path_ = "/" + strings::CleanPath(full_path_.substr(1));
  CheckPath(full_path_);
  dir_ = BuildDir(full_path_);
  build_file_ = strings::JoinPath(dir_, "BUILD");
  local_path_ = LocalPath(full_path_);
  make_path_ = strings::JoinPath(dir_, local_path_);
  top_component_ = TopComponent(dir_);
}

TargetInfo::TargetInfo(const string& relative_path,
                       const string& build_file) {
  if (strings::HasPrefix(relative_path, "//")) {
    was_relative_ = false;
    full_path_ = CleanFullPath(relative_path);
  } else {
    was_relative_ = true;
    CHECK(strings::HasSuffix(build_file, "/BUILD") || build_file == "BUILD")
        << build_file;
    full_path_ = CleanFullPath(
        "//" +
        strings::JoinPath(build_file.substr(0, build_file.size() - 5),
                          relative_path));
  }
  dir_ = BuildDir(full_path_);
  build_file_ = strings::JoinPath(dir_, "BUILD");
  local_path_ = LocalPath(full_path_);
  make_path_ = strings::JoinPath(dir_, local_path_);
  top_component_ = TopComponent(dir_);
}

TargetInfo TargetInfo::GetParallelTarget(const string& name) const {
  return TargetInfo(":" + name, build_file());
}

// static
TargetInfo TargetInfo::FromUserPath(const string& user_path) {
  if (IsValidPath(user_path)) {
    return TargetInfo(user_path);
  }

  // Prepend "//" if necessary.
  string copy = user_path;
  if (!strings::HasPrefix(user_path, "//")) {
    copy = "//" + user_path;
  }

  // Append :suffix if necessary.
  vector<StringPiece> pieces = strings::Split(copy, ":");
  if (pieces.size() == 1) {
    copy += ":" + strings::PathBasename(pieces[0]);
  }

  return TargetInfo(copy);
}

}  // namespace repobuild
