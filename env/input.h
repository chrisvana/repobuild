// Copyright 2013
// Author: Christopher Van Arsdale

#include <map>
#include <string>
#include <vector>

namespace repobuild {

class Input {
 public:
  Input(const std::string& root_dir, const std::string& object_dir)
      : root_dir_(root_dir),
        object_dir_(object_dir) {
  }
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
  const std::string& object_dir() const { return object_dir_; }
  const std::vector<std::string>& build_targets() const {
    return build_targets_;
  }
  const std::vector<std::string>& flags(const std::string& key) const;

 private:
  std::string root_dir_;
  std::string object_dir_;

  std::vector<std::string> build_targets_;
  std::map<std::string, std::vector<std::string> > flags_;
};

}  // namespace repobuild
