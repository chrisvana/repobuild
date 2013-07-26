// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_NODE_H__
#define _REPOBUILD_NODES_NODE_H__

#include <string>
#include <vector>
#include <memory>
#include "env/target.h"

namespace repobuild {

class BuildFile;
class BuildFileNode;

class Node {
 public:
  explicit Node(const TargetInfo& target)
    : target_(new TargetInfo(target)) {
  }
  virtual ~Node() {
    for (auto it : dependencies_) {
      delete it;
    }
  }

  // Virtual interface.
  virtual std::string Name() const = 0;
  virtual void Parse(const BuildFile& file, const BuildFileNode& input);

  // Accessors.
  const TargetInfo& target() const { return *target_; }
  const std::vector<TargetInfo*> dependencies() const { return dependencies_; }

 private:
  std::unique_ptr<TargetInfo> target_;
  std::vector<TargetInfo*> dependencies_;
};

}  // namespace repobuild

#endif // _REPOBUILD_NODES_NODE_H__
