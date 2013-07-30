// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_NODE_H__
#define _REPOBUILD_NODES_NODE_H__

#include <string>
#include <vector>
#include <memory>
#include "repobuild/env/target.h"

namespace repobuild {

class BuildFile;
class BuildFileNode;
class Input;

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
  virtual void WriteMakefile(const Input& input,
                             const std::vector<const Node*>& all_deps,
                             std::string* out) const = 0;
  virtual void WriteMakeClean(const Input& input, std::string* out) const {}
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void DependencyFiles(const Input& input,
                               std::vector<std::string>* files) const {}
  virtual void ObjectFiles(const Input& input,
                           std::vector<std::string>* files) const {}
  virtual void FinalOutputs(const Input& input,
                            std::vector<std::string>* outputs) const {}

  // Accessors.
  const TargetInfo& target() const { return *target_; }
  const std::vector<TargetInfo*> dependencies() const { return dependencies_; }

  // Mutators
  void AddDependency(const TargetInfo& other);

 protected:
  // Helper.
  static void ParseRepeatedString(const BuildFileNode& input,
                                  const std::string& key,
                                  std::vector<std::string>* output);
  static bool ParseStringField(const BuildFileNode& input,
                               const std::string& key,
                               std::string* field);

 private:
  std::unique_ptr<TargetInfo> target_;
  std::vector<TargetInfo*> dependencies_;
};

}  // namespace repobuild

#endif // _REPOBUILD_NODES_NODE_H__
