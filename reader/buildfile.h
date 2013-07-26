// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_READER_BUILDFILE_H__
#define _REPOBUILD_READER_BUILDFILE_H__

#include <string>
#include <vector>
#include <memory>

namespace Json {
class Value;
}

namespace repobuild {

class BuildFileNode {
 public:
  BuildFileNode(const Json::Value& object);
  ~BuildFileNode();

  const Json::Value& object() const { return *object_; }

 private:
  std::unique_ptr<Json::Value> object_;
};

class BuildFile {
 public:
  explicit BuildFile(const std::string& filename) : filename_(filename) {}
  ~BuildFile();

  // Mutators
  void Parse(const std::string& input);

  // Accessors.
  const std::string& filename() const { return filename_; }
  const std::vector<BuildFileNode*>& nodes() const { return nodes_; }

 private:
  std::string filename_;
  std::vector<BuildFileNode*> nodes_;
};

}  // namespace repobuild

#endif  // _REPOBUILD_READER_BUILDFILE_H__
