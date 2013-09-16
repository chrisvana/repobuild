// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_GEN_SH_H__
#define _REPOBUILD_NODES_GEN_SH_H__

#include <map>
#include <string>
#include <vector>
#include "repobuild/nodes/node.h"

namespace repobuild {

class GenShNode : public Node {
 public:
  GenShNode(const TargetInfo& t,
            const Input& i)
      : Node(t, i),
        cd_(true),
        make_name_("Script") {
  }
  virtual ~GenShNode() {}
  virtual std::string Name() const { return "gen_sh"; }
  virtual void Parse(BuildFile* file, const BuildFileNode& input);

  // Alternative to parse
  void Set(const std::string& build_cmd,
           const std::string& clean_cmd,
           const std::vector<Resource>& input_files,
           const std::vector<std::string>& outputs);
  void SetCd(bool cd) { cd_ = cd; }
  void SetMakeName(const std::string& name) { make_name_ = name; }
  void AddLocalEnvVariable(const std::string& var, const std::string& val) {
    local_env_vars_[var] = val;
  }

  // Static preprocessors
  static void WriteMakeHead(const Input& input, Makefile* out);

  std::string Logfile() const;

 protected:
  std::string WriteCommand(const std::map<std::string, std::string>& env_vars,
                           const std::string& prefix,
                           const std::string& cmd,
                           const std::string& admin_cmd) const;

  virtual void LocalWriteMakeClean(Makefile* out) const;
  virtual void LocalWriteMake(Makefile* out) const;
  virtual void LocalDependencyFiles(LanguageType lang,
                                    ResourceFileSet* files) const;

  // NB: We intentionally do not pass on sub-dependency files, and rely soley
  // on our "touchfile'.
  virtual bool IncludeDependencies(DependencyCollectionType type,
                                   LanguageType lang) const {
    return (type != DEPENDENCY_FILES);
  }

  std::string build_cmd_;
  std::string clean_cmd_;
  std::vector<Resource> input_files_;
  std::vector<std::string> outputs_;
  std::map<std::string, std::string> local_env_vars_;
  bool cd_;
  std::string make_name_;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_GEN_SH_H__
