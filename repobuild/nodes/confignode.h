// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_CONFIGNODE_H__
#define _REPOBUILD_NODES_CONFIGNODE_H__

#include <memory>
#include <string>
#include "repobuild/nodes/node.h"
#include "repobuild/env/resource.h"

namespace repobuild {
class ComponentHelper;

class ConfigNode : public Node {
 public:
  ConfigNode(const TargetInfo& t, const Input& i, DistSource* source);
  virtual ~ConfigNode();
  virtual std::string Name() const { return "config"; }
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void LocalWriteMakeClean(Makefile::Rule* out) const;
  virtual void LocalWriteMake(Makefile* out) const;
  virtual void LocalDependencyFiles(LanguageType lang,
                                    ResourceFileSet* files) const;
  virtual void LocalIncludeDirs(LanguageType lang,
                                std::set<std::string>* dirs) const;
  virtual bool PathRewrite(std::string* output_path,
                           std::string* rewrite_root) const;

 protected:
  void AddSymlink(const std::string& dir,
                  const std::string& source,
                  Makefile* out) const;
  std::string DummyFile(const std::string& dir) const;
  std::string SourceDir(const std::string& middle) const;

  std::unique_ptr<ComponentHelper> component_;
  Resource source_dummy_file_, gendir_dummy_file_, pkgfile_dummy_file_;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_CONFIGNODE_H__
