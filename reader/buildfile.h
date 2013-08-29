// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_READER_BUILDFILE_H__
#define _REPOBUILD_READER_BUILDFILE_H__

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace Json {
class Value;
}

namespace repobuild {

class BuildFileNode {
 public:
  BuildFileNode(const Json::Value& object);
  ~BuildFileNode();

  // Data source
  const Json::Value& object() const { return *object_; }

 private:
  std::unique_ptr<Json::Value> object_;
};

class BuildFile {
 public:
  explicit BuildFile(const std::string& filename)
      : filename_(filename),
        name_counter_(0) {
  }
  ~BuildFile();

  // Mutators
  void Parse(const std::string& input);
  void MergeParent(BuildFile* parent);
  void AddBaseDependency(const std::string& dep) { base_deps_.insert(dep); }

  // Accessors.
  const std::string& filename() const { return filename_; }
  const std::vector<BuildFileNode*>& nodes() const { return nodes_; }
  const std::set<std::string>& base_dependencies() const { return base_deps_; }

  // Helpers.
  std::string NextName();  // auto generated target name.

 private:
  std::string filename_;
  std::vector<BuildFileNode*> nodes_;
  std::set<std::string> base_deps_;
  int name_counter_;
};

}  // namespace repobuild

#endif  // _REPOBUILD_READER_BUILDFILE_H__
