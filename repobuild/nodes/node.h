// Copyright 2013
// Author: Christopher Van Arsdale
//
// Our BUILD entries are encoded internally as "Nodes" below. This class is
// the base class for all node types. For example:
// cc_library -> CCLibraryNode (cc_library.cc), which inherits from Node below.

#ifndef _REPOBUILD_NODES_NODE_H__
#define _REPOBUILD_NODES_NODE_H__

#include <list>
#include <map>
#include <memory>
#include <string>
#include <set>
#include <utility>
#include <vector>
#include "repobuild/env/resource.h"
#include "repobuild/env/target.h"
#include "repobuild/nodes/makefile.h"
#include "repobuild/reader/buildfile.h"
#include "common/strings/strutil.h"

namespace repobuild {
class ComponentHelper;
class DistSource;
class Input;

class Node {
 public:
  // TODO(cvanarsdale): This is hacky.
  enum LanguageType {
    C_LANG = 0,
    CPP = 1,
    JAVA = 2,
    PYTHON = 3,
    GO_LANG = 4,
    NO_LANG = 5,
  };

  Node(const TargetInfo& target, const Input& input, DistSource* source);
  virtual ~Node();

  // Initialization
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void PostParse();

  // Makefile generation.
  void WriteMake(Makefile* out) const;
  void WriteMakeClean(Makefile::Rule* rule) const;
  void WriteMakeInstall(Makefile* base, Makefile::Rule* rule) const;

  // Internal object/resource handling.
  void DependencyFiles(LanguageType lang, ResourceFileSet* files) const;
  void ObjectFiles(LanguageType lang, ResourceFileSet* files) const;
  void ObjectRoots(LanguageType lang, ResourceFileSet* dirs) const;
  void FinalOutputs(LanguageType lang, ResourceFileSet* outputs) const;
  void FinalTests(LanguageType lang, std::set<std::string>* targets) const;
  void Binaries(LanguageType lang, ResourceFileSet* outputs) const;
  void TopTestBinaries(LanguageType lang, ResourceFileSet* outputs) const;
  void SystemDependencies(LanguageType lang, std::set<std::string>* deps) const;
  virtual void ExternalDependencyFiles(
      LanguageType lang,
      std::map<std::string, std::string>* files) const {}
  virtual bool IncludeInAll() const { return true; }
  virtual bool IncludeInTests() const { return false; }

  // Flag inheritence
  void LinkFlags(LanguageType lang,
                 std::set<std::string>* flags) const;
  void CompileFlags(LanguageType lang,
                    std::set<std::string>* flags) const;
  void IncludeDirs(LanguageType lang,
                   std::set<std::string>* dirs) const;
  void EnvVariables(LanguageType lang,
                    std::map<std::string, std::string>* vars) const;

  // Accessors.
  const Input& input() const { return *input_; }
  const TargetInfo& target() const { return target_; }
  const std::vector<TargetInfo> dep_targets() const { return dep_targets_; }
  const std::vector<TargetInfo> required_parents() const {
    return required_parents_;
  }
  const std::vector<TargetInfo> pre_parse() const { return pre_parse_; }
  const std::vector<Node*> dependencies() const { return dependencies_; }
  DistSource* dist_source() const { return dist_source_; }

  // Mutators
  void AddDependencyNode(Node* dependency);
  void AddDependencyTarget(const TargetInfo& other);
  void AddRequiredParent(const TargetInfo& parent);
  void AddPreParse(const TargetInfo& other);
  void CopyDepenencies(Node* other);
  void SetStrictFileMode(bool strict) { strict_file_mode_ = strict; }

  // Subnode handling.
  TargetInfo GetNextTargetName(BuildFile* file) const;
  void ExtractSubnodes(std::vector<Node*>* nodes);
  void AddSubNode(Node* node);
  template <class T> T* NewSubNode(BuildFile* file);
  template <class T> T* NewSubNodeWithCurrentDeps(BuildFile* file);

  // Plugin stuff
  virtual bool ExpandBuildFileNode(BuildFile* file, BuildFileNode* node) {
    return false;
  }

 protected:
  class MakeVariable;

  // The main thing to override.
  virtual void LocalWriteMake(Makefile* out) const = 0;
  virtual void LocalWriteMakeClean(Makefile::Rule* out) const {}
  virtual void LocalWriteMakeInstall(Makefile* base,
                                     Makefile::Rule* out) const {
  }
  virtual void LocalDependencyFiles(
      LanguageType lang,
      ResourceFileSet* files) const {}
  virtual void LocalObjectFiles(
      LanguageType lang,
      ResourceFileSet* files) const {}
  virtual void LocalObjectRoots(
      LanguageType lang,
      ResourceFileSet* dirs) const {}
  virtual void LocalSystemDependencies(
      LanguageType lang,
      std::set<std::string>* deps) const {}
  virtual void LocalFinalOutputs(
      LanguageType lang,
      ResourceFileSet* outputs) const {}
  virtual void LocalTests(
      LanguageType lang,
      std::set<std::string>* targets) const {}
  virtual void LocalBinaries(
      LanguageType lang,
      ResourceFileSet* outputs) const {}
  virtual void LocalLinkFlags(
      LanguageType lang,
      std::set<std::string>* flags) const {}
  virtual void LocalCompileFlags(
      LanguageType lang,
      std::set<std::string>* flags) const {}
  virtual void LocalIncludeDirs(
      LanguageType lang,
      std::set<std::string>* dirs) const {}
  virtual void LocalEnvVariables(
      LanguageType lang, 
      std::map<std::string, std::string>* vars) const;
  virtual bool PathRewrite(std::string* output_path,
                           std::string* rewrite_root) const {
    return false;
  }

  // Parsing helpers
  BuildFileNodeReader* NewBuildReader(const BuildFileNode& node) const;
  BuildFileNodeReader* current_reader() const { return build_reader_.get(); }

  // Directory helpers.
  std::string GenDir() const { return gen_dir_; }
  std::string RelativeGenDir() const { return relative_gen_dir_; }
  std::string ObjectDir() const { return obj_dir_; }
  std::string RelativeObjectDir() const { return relative_obj_dir_; }
  std::string SourceDir() const { return src_dir_; }
  std::string RelativeSourceDir() const { return relative_src_dir_; }
  std::string PackageDir() const { return package_dir_; }
  std::string RelativeRootDir() const { return relative_root_dir_; }
  std::string StripSpecialDirs(const std::string& path) const;

  // Makefile helpers.
  Resource Touchfile(const std::string& suffix) const;
  Resource Touchfile() const { return Touchfile(""); }
  void WriteBaseUserTarget(const ResourceFileSet& deps, Makefile* out) const;
  void WriteBaseUserTarget(Makefile* out) const;
  void WriteVariables(std::string* out) const;
  bool HasVariable(const std::string& name) const;
  const MakeVariable& GetVariable(const std::string& name) const;
  MakeVariable* MutableVariable(const std::string& name);
  void AddConditionalVariable(const std::string& var_name,
                              const std::string& condition_name,
                              const std::string& true_value,
                              const std::string& false_value);

  // Dependency helpers
  void InputDependencyFiles(LanguageType lang, ResourceFileSet* files) const;
  void InputObjectFiles(LanguageType lang, ResourceFileSet* files) const;
  void InputObjectRoots(LanguageType lang, ResourceFileSet* dirs) const;
  void InputSystemDependencies(LanguageType lang,
                               std::set<std::string>* deps) const;
  void InputFinalOutputs(LanguageType lang, ResourceFileSet* outputs) const;
  void InputTests(LanguageType lang, std::set<std::string>* targets) const;
  void InputBinaries(LanguageType lang, ResourceFileSet* outputs) const;
  void InputLinkFlags(LanguageType lang, std::set<std::string>* flags) const;
  void InputCompileFlags(LanguageType lang, std::set<std::string>* flags) const;
  void InputIncludeDirs(LanguageType lang, std::set<std::string>* dirs) const;
  void InputEnvVariables(LanguageType lang,
                         std::map<std::string, std::string>* vars) const;
  void InitComponentHelpers();
  const ComponentHelper* GetComponentHelper(const std::string& path) const;
  const ComponentHelper* GetComponentHelper(const ComponentHelper* preferred,
                                            const std::string& path) const;

  enum DependencyCollectionType {
    DEPENDENCY_FILES,
    OBJECT_FILES,
    SYSTEM_DEPENDENCIES,
    FINAL_OUTPUTS,
    BINARIES,
    TESTS,
    LINK_FLAGS,
    COMPILE_FLAGS,
    INCLUDE_DIRS,
    ENV_VARIABLES
  };
  void CollectAllDependencies(DependencyCollectionType type,
                              LanguageType lang,
                              std::vector<Node*>* all_deps) const;
  virtual bool IncludeDependencies(DependencyCollectionType type,
                                   LanguageType lang) const {
    return true;
  }
  virtual bool IncludeChildDependency(DependencyCollectionType type,
                                      LanguageType lang,
                                      Node* node) const {
    return true;
  }

 private:
  void CollectAllDependencies(DependencyCollectionType type,
                              LanguageType lang,
                              std::set<Node*>* all_deps_set,
                              std::vector<Node*>* all_deps) const;

  // Input info.
  TargetInfo target_;
  const Input* input_;
  DistSource* dist_source_;
  std::vector<TargetInfo> dep_targets_, required_parents_, pre_parse_;
  std::string src_dir_, obj_dir_, gen_dir_, package_dir_;
  std::string relative_root_dir_, relative_src_dir_;
  std::string relative_obj_dir_, relative_gen_dir_;

  // Parsing info
  bool strict_file_mode_;
  std::unique_ptr<BuildFileNodeReader> build_reader_;
  std::map<std::string, std::string> env_variables_;

  // Subnode/variables/etc handling.
  std::vector<Node*> subnodes_, owned_subnodes_;
  std::vector<Node*> dependencies_;  // not owned.
  std::map<std::string, MakeVariable*> make_variables_;

  // File path handling
  std::vector<ComponentHelper*> component_helpers_;
};

class Node::MakeVariable {
 public:
  explicit MakeVariable(const std::string& name);
  ~MakeVariable();

  // Accessors.
  const std::string& name() const;
  std::string ref_name() const;

  // Mutators
  void SetValue(const std::string& value);
  void SetCondition(const std::string& condition,
                    const std::string& if_val,
                    const std::string& else_val);

  // Makefile generation.
  void WriteMake(std::string* out) const;

 private:
  std::string name_;
  std::map<std::string, std::pair<std::string, std::string> > conditions_;
};

template <class T>
T* Node::NewSubNode(BuildFile* file) {
  T* node = new T(GetNextTargetName(file), input(), dist_source_);
  AddSubNode(node);
  return node;
}

template <class T>
T* Node::NewSubNodeWithCurrentDeps(BuildFile* file) {
  T* node = new T(GetNextTargetName(file), input(), dist_source_);
  node->CopyDepenencies(this);
  AddSubNode(node);
  return node;
}

}  // namespace repobuild

#endif // _REPOBUILD_NODES_NODE_H__
