// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_NODE_H__
#define _REPOBUILD_NODES_NODE_H__

#include <map>
#include <memory>
#include <string>
#include <set>
#include <utility>
#include <vector>
#include "repobuild/env/target.h"

namespace repobuild {

class BuildFile;
class BuildFileNode;
class Input;

class Node {
 public:
  explicit Node(const TargetInfo& target,
                const Input& input)
      : target_(target),
        input_(&input),
        strict_file_mode_(true) {
  }
  virtual ~Node() {
    for (auto it : dependencies_) {
      delete it;
    }
    for (auto it : owned_subnodes_) {
      delete it;
    }
  }

  // Virtual interface.
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void WriteMake(const std::vector<const Node*>& all_deps,
                         std::string* out) const;
  virtual void WriteMakeClean(std::string* out) const {}
  virtual void DependencyFiles(std::vector<std::string>* files) const {}
  virtual void ObjectFiles(std::vector<std::string>* files) const {}
  virtual void FinalOutputs(std::vector<std::string>* outputs) const {}
  virtual void LinkFlags(std::set<std::string>* flags) const {}
  virtual void CompileFlags(bool cxx, std::set<std::string>* flags) const {}

  // Accessors.
  const Input& input() const { return *input_; }
  const TargetInfo& target() const { return target_; }
  const std::vector<TargetInfo*> dependencies() const { return dependencies_; }

  // Mutators
  void AddDependency(const TargetInfo& other);
  void SetStrictFileMode(bool strict) { strict_file_mode_ = strict; }

  // Subnode handling.
  void ExtractSubnodes(std::vector<Node*>* nodes) {
    *nodes = subnodes_;
    owned_subnodes_.clear();
  }
  void AddSubNode(Node* node) {
    AddDependency(node->target());
    subnodes_.push_back(node);
    owned_subnodes_.push_back(node);
  }

 protected:
  class MakeVariable {
   public:
    explicit MakeVariable(const std::string& name) : name_(name) {}
    ~MakeVariable() {}

    const std::string& name() const { return name_; }
    std::string ref_name() const {
      return name_.empty() ? "" : "$(" + name_ + ")";
    }
    void SetValue(const std::string& value) { SetCondition("", value, ""); }
    void SetCondition(const std::string& condition,
                      const std::string& if_val,
                      const std::string& else_val) {
      conditions_[condition] = std::make_pair(if_val, else_val);
    }
    void WriteMake(std::string* out) const;

   private:
    std::string name_;
    std::map<std::string, std::pair<std::string, std::string> > conditions_;
  };

  // The main thing to override.
  virtual void WriteMakefile(const std::vector<const Node*>& all_deps,
                             std::string* out) const = 0;

  // Helpers
  void ParseRepeatedString(const BuildFileNode& input,
                           const std::string& key,
                           std::vector<std::string>* output) const {
    ParseRepeatedString(input, key, false /* use root path */, output);
  }
  void ParseRepeatedString(const BuildFileNode& input,
                           const std::string& key,
                           bool relative_gendir,
                           std::vector<std::string>* output) const;
  void ParseRepeatedFiles(const BuildFileNode& input,
                          const std::string& key,
                          std::vector<std::string>* output) const;
  bool ParseStringField(const BuildFileNode& input,
                        const std::string& key,
                        std::string* field) const;
  bool ParseBoolField(const BuildFileNode& input,
                      const std::string& key,
                      bool* field) const;
  void CollectDependencies(const std::vector<const Node*>& all_deps,
                           std::set<std::string>* files) const;
  void CollectObjects(const std::vector<const Node*>& all_deps,
                      std::set<std::string>* files) const;
  void CollectLinkFlags(const std::vector<const Node*>& all_deps,
                        std::set<std::string>* flags) const;
  void CollectCompileFlags(bool cxx,
                           const std::vector<const Node*>& all_deps,
                           std::set<std::string>* flags) const;
  std::string ParseSingleString(const std::string& input) const {
    return ParseSingleString(true, input);
  }
  std::string ParseSingleString(bool relative_gendir,
                                const std::string& input) const;
  std::string GenDir() const;
  std::string RelativeGenDir() const;
  std::string ObjectDir() const;
  std::string RelativeObjectDir() const;
  std::string MakefileEscape(const std::string& str) const;
  std::string WriteBaseUserTarget(const std::set<std::string>& deps) const;

  void WriteVariables(std::string* out) const {
    for (auto const& it : make_variables_) {
      it.second->WriteMake(out);
    }
  }
  bool HasVariable(const std::string& name) const {
    return make_variables_.find(name) != make_variables_.end();
  }
  const MakeVariable& GetVariable(const std::string& name) const {
    static MakeVariable kEmpty("");
    const auto& it = make_variables_.find(name);
    if (it == make_variables_.end()) {
      return kEmpty;
    }
    return *it->second;
  }
  MakeVariable* MutableVariable(const std::string& name) {
    MakeVariable** var = &(make_variables_[name]);
    if (*var == NULL) {
      *var = new MakeVariable(name + "." + target().make_path());
    }
    return *var;
  }

 private:
  TargetInfo target_;
  const Input* input_;
  std::vector<TargetInfo*> dependencies_;
  bool strict_file_mode_;

  std::vector<Node*> subnodes_, owned_subnodes_;
  std::map<std::string, MakeVariable*> make_variables_;
};

}  // namespace repobuild

#endif // _REPOBUILD_NODES_NODE_H__
