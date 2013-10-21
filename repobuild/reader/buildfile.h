// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_READER_BUILDFILE_H__
#define _REPOBUILD_READER_BUILDFILE_H__

#include <memory>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "common/base/macros.h"
#include "repobuild/env/target.h"

namespace Json {
class Value;
}
namespace strings {
class VarMap;
}

namespace repobuild {
class DistSource;
class Resource;

class BuildFileNode {
 public:
  explicit BuildFileNode(const Json::Value& object);
  ~BuildFileNode();

  // Data source
  const Json::Value& object() const { return *object_; }

  // Mutators
  void Reset(const Json::Value& object);

 private:
  std::unique_ptr<Json::Value> object_;
};

class BuildFile {
 public:
  explicit BuildFile(const std::string& filename)
      : filename_(filename) {
  }
  ~BuildFile();

  // Mutators
  void Parse(const std::string& input);
  void MergeParent(BuildFile* parent);
  void MergeDependency(BuildFile* dependency);
  void AddBaseDependency(const std::string& dep) { base_deps_.insert(dep); }
  void RegisterKey(const std::string& key, const std::string& value) {
    registered_keys_[key] = value;
  }

  // Dependency rewriting.
  class BuildDependencyRewriter {
   public:
    BuildDependencyRewriter() {}
    virtual ~BuildDependencyRewriter() {}
    virtual bool RewriteDependency(TargetInfo* target) = 0;
  };
  void AddDependencyRewriter(BuildDependencyRewriter* rewriter) {
    owned_rewriters_.push_back(rewriter);
    rewriters_.push_back(rewriter);
  }

  // Accessors.
  const std::string& filename() const { return filename_; }
  const std::vector<BuildFileNode*>& nodes() const { return nodes_; }
  const std::set<std::string>& base_dependencies() const { return base_deps_; }
  const std::string GetKey(const std::string& key) const;

  // Helpers.
  std::string NextName(const std::string& name_base);  // auto generated name.
  TargetInfo ComputeTargetInfo(const std::string& dependency) const;

 private:
  std::string filename_;
  std::vector<BuildFileNode*> nodes_;
  std::set<std::string> base_deps_;
  std::map<std::string, int> name_counter_;
  std::vector<BuildDependencyRewriter*> owned_rewriters_, rewriters_;
  std::map<std::string, std::string> registered_keys_;
};

// BuildFileNodeReader
//  Helper that makes it easier to parse a BuildFileNode.
class BuildFileNodeReader {
 public:
  BuildFileNodeReader(const BuildFileNode& node,
                      DistSource* dist_source);
  ~BuildFileNodeReader();

  // Mutators
  void SetReplaceVariable(bool mode,
                          const std::string& original,
                          const std::string& replace);
  void AddFileAbsPrefix(const std::string& pre) { abs_prefix_.insert(pre); }
  void SetStrictFileMode(bool file_mode) { strict_file_mode_ = file_mode; }
  void SetErrorPath(const std::string& path) { error_path_ = path; }
  void SetFilePath(const std::string& path) { file_path_ = path; }

  // Parse strings.
  void ParseRepeatedString(const std::string& key,
                           std::vector<std::string>* output) const {
    ParseRepeatedString(key, false, output);
  }
  void ParseRepeatedString(const std::string& key,
                           bool mode,
                           std::vector<std::string>* output) const;
  void ParseKeyValueStrings(const std::string& key,
                            std::map<std::string, std::string>* output) const;
  bool ParseStringField(const std::string& key,
                        std::string* field) const;
  bool ParseStringField(const std::string& key,
                        bool mode,
                        std::string* field) const;

  // Parse files.
  void ParseRepeatedFiles(const std::string& key,
                          std::vector<Resource>* output) const {
    ParseRepeatedFiles(key, strict_file_mode_, output);
  }
  void ParseRepeatedFiles(const std::string& key,
                          bool strict_file_mode,
                          std::vector<Resource>* output) const;
  void ParseSingleFile(const std::string& key,
                       std::vector<Resource>* output) const {
    ParseSingleFile(key, strict_file_mode_, output);
  }
  void ParseSingleFile(const std::string& key,
                       bool strict_file_mode,
                       std::vector<Resource>* output) const;

  // Parsing a single directory
  std::string ParseSingleDirectory(const std::string& key) const;
  std::string ParseSingleDirectory(bool strict_file_mode,
                                   const std::string& key) const;

  // Parse bool.
  bool ParseBoolField(const std::string& key,
                      bool* field) const;

 private:
  void ParseFilesFromString(const std::vector<std::string>& input,
                            bool strict_file_mode,
                            std::vector<Resource>* output) const;

  DISALLOW_COPY_AND_ASSIGN(BuildFileNodeReader);

  std::string RewriteSingleString(bool mode, const std::string& str) const;

  const BuildFileNode& input_;
  DistSource* dist_source_;
  std::unique_ptr<strings::VarMap> var_map_true_;
  std::unique_ptr<strings::VarMap> var_map_false_;
  std::set<std::string> abs_prefix_;
  bool strict_file_mode_;
  std::string error_path_;
  std::string file_path_;
};

}  // namespace repobuild

#endif  // _REPOBUILD_READER_BUILDFILE_H__
