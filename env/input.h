// Copyright 2013
// Author: Christqopher Van Arsdale

#ifndef _REPOBUILD_ENV_INPUT__
#define _REPOBUILD_ENV_INPUT__

#include <map>
#include <string>
#include <vector>
#include <set>
#include "repobuild/env/target.h"

namespace repobuild {

class Input {
 public:
  Input();
  ~Input() {}

  // Mutators:
  void AddBuildTarget(const TargetInfo& target);
  void AddFlag(const std::string& key, const std::string& value) {
    flags_[key].push_back(value);
  }

  // Accessors
  const std::string& root_dir() const { return root_dir_; }
  const std::string& full_root_dir() const { return full_root_dir_; }
  const std::string& object_dir() const { return object_dir_; }
  const std::string& full_object_dir() const { return full_object_dir_; }
  const std::string& genfile_dir() const { return genfile_dir_; }
  const std::string& full_genfile_dir() const { return full_genfile_dir_; }
  const std::string& source_dir() const { return source_dir_; }
  const std::vector<TargetInfo>& build_targets() const {
    return build_targets_;
  }
  const std::vector<std::string>& flags(const std::string& key) const;
  bool contains_target(const std::string& target) const {
    return build_target_set_.find(target) != build_target_set_.end();
  }
  bool silent_make() const { return silent_make_; }

  // proto_library stuff.
  const std::string& default_cc_proto() const { return default_cc_proto_; }
  const std::string& default_java_proto() const { return default_java_proto_; }
  const std::string& default_go_proto() const { return default_go_proto_; }
  const std::string& default_py_proto() const { return default_py_proto_; }

 private:
  std::string root_dir_, full_root_dir_;
  std::string current_path_;
  std::string object_dir_, full_object_dir_;
  std::string genfile_dir_, full_genfile_dir_;
  std::string source_dir_;

  std::vector<TargetInfo> build_targets_;
  std::set<std::string> build_target_set_;
  std::map<std::string, std::vector<std::string> > flags_;

  bool silent_make_;

  std::string default_cc_proto_;
  std::string default_java_proto_;
  std::string default_go_proto_;
  std::string default_py_proto_;
};

}  // namespace repobuild

#endif  // _REPOBUILD_ENV_INPUT__
