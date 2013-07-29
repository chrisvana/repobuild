// Copyright 2013
// Author: Christopher Van Arsdale

#include <map>
#include <string>
#include <vector>

namespace repobuild {

class Input {
 public:
  Input();
  ~Input() {}

  // Mutators:
  void AddBuildTarget(const std::string& target) {
    build_targets_.push_back(target);
  }
  void AddFlag(const std::string& key,  const std::string& value) {
    flags_[key].push_back(value);
  }

  // Accessors
  const std::string& root_dir() const { return root_dir_; }
  const std::string& full_root_dir() const { return full_root_dir_; }
  const std::string& object_dir() const { return object_dir_; }
  const std::string& full_object_dir() const { return full_object_dir_; }
  const std::string& source_dir() const { return source_dir_; }
  const std::vector<std::string>& build_targets() const {
    return build_targets_;
  }
  const std::vector<std::string>& flags(const std::string& key) const;

 private:
  std::string root_dir_, full_root_dir_;
  std::string current_path_;
  std::string object_dir_, full_object_dir_;
  std::string source_dir_;

  std::vector<std::string> build_targets_;
  std::map<std::string, std::vector<std::string> > flags_;
};

}  // namespace repobuild
