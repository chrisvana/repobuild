// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_ENV_TARGET__
#define _REPOBUILD_ENV_TARGET__

#include <string>

namespace repobuild {

class TargetInfo {
 public:
  TargetInfo() {}  // for stl, do not use.
  explicit TargetInfo(const std::string& full_path);
  TargetInfo(const std::string& relative_path, const std::string& build_file);

  ~TargetInfo() {}

  const std::string& full_path() const { return full_path_; }
  const std::string& build_file() const { return build_file_; }
  const std::string& dir() const { return dir_; }
  const std::string& local_path() const { return local_path_; }
  const std::string& make_path() const { return make_path_; }

  bool operator==(const TargetInfo& other) const {
    return full_path_ == other.full_path_;
  }

  // Helpers
  TargetInfo GetParallelTarget(const std::string& name) const;
  static TargetInfo FromUserPath(const std::string& user_path);

 private:
  std::string full_path_, build_file_, dir_, local_path_, make_path_;
};

}  // namespace repobuild

#endif  //  _REPOBUILD_ENV_TARGET__
