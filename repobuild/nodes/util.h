// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_UTIL_H__
#define _REPOBUILD_NODES_UTIL_H__

#include <string>

namespace repobuild {
class Input;
class TargetInfo;

class NodeUtil {
 public: 
  static std::string StripSpecialDirs(const Input& input,
                                      const std::string& path);
};

class ComponentHelper {
 public:
  ComponentHelper(const std::string& component,
                  const std::string& base_dir);
  ~ComponentHelper() {}

  const std::string& component() const { return component_; }
  const std::string& base_dir() const { return base_dir_; }

  bool CoversPath(const Input& input, const std::string& path) const;
  std::string RewriteFile(const Input& input, const std::string& path) const;
  bool RewriteDependency(TargetInfo* target) const;

  ComponentHelper* Clone() const {
    return new ComponentHelper(component_, base_dir_);
  }

 private:
  std::string component_, base_dir_;
};

}  // namespace repobuild

#endif  // _REPOBUILD_NODES_UTIL_H__
