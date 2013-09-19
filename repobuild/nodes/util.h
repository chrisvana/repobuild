// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_UTIL_H__
#define _REPOBUILD_NODES_UTIL_H__

#include <string>

namespace repobuild {
class Input;

class NodeUtil {
 public: 
  static std::string StripSpecialDirs(const Input& input,
                                      const std::string& path);
};

}  // namespace repobuild

#endif  // _REPOBUILD_NODES_UTIL_H__
