// Copyright 2013
// Author: Christopher Van Arsdale

#include <string>

namespace repobuild {

class TargetInfo {
 public:
  explicit TargetInfo(const std::string& full_path);
  TargetInfo(const std::string& relative_path, const std::string& build_file);
  TargetInfo(const TargetInfo& other)
      : full_path_(other.full_path_),
        build_file_(other.build_file_) {
  }

  ~TargetInfo() {}

  const std::string& full_path() const { return full_path_; }
  const std::string& build_file() const { return build_file_; }

 private:
  std::string full_path_, build_file_;
};

}  // namespace repobuild
