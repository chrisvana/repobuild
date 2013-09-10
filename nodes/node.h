// Copyright 2013
// Author: Christopher Van Arsdale

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
#include "common/strings/strutil.h"

namespace repobuild {

class BuildFile;
class BuildFileNode;
class BuildFileNodeReader;
class Input;

class Makefile;

class Node {
 public:
  class ObjectFileSet {
   public:
    ObjectFileSet() {}
    ~ObjectFileSet() {}

    const std::list<Resource>& files() const { return files_; }
    void Add(const Resource& resource) {
      // HACK(cvanarsdale):
      // Sadly order matters to the (gcc) linker. It looks in later object
      // files to find unresolved symbols. We collect the dependencies
      // bottom up, so we push resources onto the front of the list so
      // unencumbered resources end up in the back of the list.
      if (fileset_.insert(resource).second) {
        files_.push_front(resource);
      }
    }

   private:
    std::set<Resource> fileset_;
    std::list<Resource> files_;
  };

  Node(const TargetInfo& target, const Input& input);
  virtual ~Node();

  // Virtual interface.
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void WriteMake(const std::vector<const Node*>& all_deps,
                         Makefile* out) const;
  virtual void WriteMakeClean(const std::vector<const Node*>& all_deps,
                              Makefile* out) const {}
  virtual void DependencyFiles(std::set<Resource>* files) const;
  virtual void ObjectFiles(ObjectFileSet* files) const;
  virtual void FinalOutputs(std::set<Resource>* outputs) const;
  virtual void LinkFlags(std::set<std::string>* flags) const;
  virtual void CompileFlags(bool cxx, std::set<std::string>* flags) const;
  virtual void EnvVariables(std::map<std::string, std::string>* vars) const;

  // Accessors.
  const Input& input() const { return *input_; }
  const TargetInfo& target() const { return target_; }
  const std::vector<TargetInfo> dep_targets() const { return dep_targets_; }
  const std::vector<Node*> dependencies() const { return dependencies_; }

  // Mutators
  void AddDependencyTarget(const TargetInfo& other);
  void SetStrictFileMode(bool strict) { strict_file_mode_ = strict; }

  // Called from outside:
  void AddDependencyNode(Node* dependency);

  // Subnode handling.
  void ExtractSubnodes(std::vector<Node*>* nodes) {
    for (Node* n : subnodes_) {
      nodes->push_back(n);
      n->ExtractSubnodes(nodes);
    }
    owned_subnodes_.clear();
  }
  void AddSubNode(Node* node) {
    AddDependencyTarget(node->target());
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
                             Makefile* out) const = 0;

  // Parsing helpers
  BuildFileNodeReader* NewBuildReader(const BuildFileNode& node) const;
  BuildFileNodeReader* current_reader() const { return build_reader_.get(); }

  // Directory helpers.
  std::string GenDir() const;
  std::string RelativeGenDir() const;
  std::string ObjectDir() const;
  std::string RelativeObjectDir() const;
  std::string SourceDir() const;
  std::string RelativeSourceDir() const;
  std::string RelativeRootDir() const;
  std::string GetRelative(const std::string& path) const;

  // Makefile helpers.
  std::string MakefileEscape(const std::string& str) const;
  Resource Touchfile(const std::string& suffix) const;
  Resource Touchfile() const { return Touchfile(""); }
  void WriteBaseUserTarget(const std::set<Resource>& deps,
                           Makefile* out) const;
  void WriteBaseUserTarget(Makefile* out) const {
    std::set<Resource> empty;
    WriteBaseUserTarget(empty, out);
  }
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
  // Input info.
  TargetInfo target_;
  const Input* input_;
  std::vector<TargetInfo> dep_targets_;

  // Parsing info
  bool strict_file_mode_;
  std::unique_ptr<BuildFileNodeReader> build_reader_;
  std::map<std::string, std::string> env_variables_;

  // Subnode/variables/etc handling.
  std::vector<Node*> subnodes_, owned_subnodes_;
  std::vector<Node*> dependencies_;  // not owned.
  std::map<std::string, MakeVariable*> make_variables_;
};

// SimpleLibraryNode
//  A library that just collects dependencies.
class SimpleLibraryNode : public Node {
 public:
  SimpleLibraryNode(const TargetInfo& t, const Input& i) : Node(t, i) {}
  virtual ~SimpleLibraryNode() {}
  virtual void WriteMakefile(const std::vector<const Node*>& all_deps,
                             Makefile* out) const {}
  virtual void DependencyFiles(std::set<Resource>* files) const;
  virtual void ObjectFiles(ObjectFileSet* files) const;

  // Alterative to Parse()
  virtual void Set(const std::vector<Resource>& sources) {
    sources_ = sources;
  }

 protected:
  std::vector<Resource> sources_;
};

class Makefile {
 public:
  Makefile() :silent_(true) {}
  ~Makefile() {}

  // Options
  void SetSilent(bool silent) { silent_ = silent; }

  // Rules
  // TODO(cvanarsdale): Return a pointer to a MakefileRule.
  void StartRule(const std::string& rule) { StartRule(rule, ""); }
  void StartRule(const std::string& rule, const std::string& dependencies);
  void FinishRule();
  void WriteRule(const std::string& rule, const std::string& deps) {
    StartRule(rule, deps);
    FinishRule();
  }

  // Commands for rules.
  void WriteCommand(const std::string& command);
  void WriteCommandBestEffort(const std::string& command);

  std::string* mutable_out() { return &out_; }
  const std::string& out() const { return out_; }

  template <typename T>
  void append(const T& t) {
    out_.append(strings::StringPrint(t));
  }

 private:
  bool silent_;
  std::string out_;
};


}  // namespace repobuild

#endif // _REPOBUILD_NODES_NODE_H__
