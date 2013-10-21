// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_JAVA_LIBRARY_H__
#define _REPOBUILD_NODES_JAVA_LIBRARY_H__

#include <memory>
#include <set>
#include <string>
#include <vector>
#include "repobuild/env/resource.h"
#include "repobuild/nodes/node.h"

namespace repobuild {
class ComponentHelper;

class JavaLibraryNode : public Node {
 public:
  JavaLibraryNode(const TargetInfo& t,
                  const Input& i,
                  DistSource* s);
  virtual ~JavaLibraryNode();
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void LocalWriteMake(Makefile* out) const {
    LocalWriteMakeInternal(true, out);
  }
  virtual void LocalObjectFiles(LanguageType lang,
                                ResourceFileSet* files) const;
  virtual void LocalObjectRoots(LanguageType lang,
                                ResourceFileSet* dirs) const;
  virtual void LocalLinkFlags(LanguageType lang,
                              std::set<std::string>* flags) const;
  virtual void LocalCompileFlags(LanguageType lang,
                                 std::set<std::string>* flags) const;
  virtual void LocalIncludeDirs(LanguageType lang,
                                std::set<std::string>* dirs) const;
  virtual void LocalDependencyFiles(LanguageType lang,
                                    ResourceFileSet* files) const;

  // For direct construction.
  void Set(BuildFile* file,
           const BuildFileNode& input,
           const std::vector<Resource>& sources);

 protected:
  void ParseInternal(BuildFile* file, const BuildFileNode& input);
  void LocalWriteMakeInternal(bool write_user_target, Makefile* out) const;
  void WriteCompile(const ResourceFileSet& input_files,
                    Makefile* out) const;
  Resource ClassFile(const Resource& source) const;
  Resource ObjectRoot() const;
  Resource RootTouchfile() const;

  std::vector<Resource> sources_;
  std::vector<std::string> java_local_compile_args_;
  std::vector<std::string> java_compile_args_;
  std::vector<std::string> java_jar_args_;
  std::vector<std::string> java_classpath_;
  std::string java_out_root_;

  std::unique_ptr<ComponentHelper> component_;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_JAVA_LIBRARY_H__
