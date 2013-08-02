// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_GEN_SH_H__
#define _REPOBUILD_NODES_GEN_SH_H__

#include <string>
#include <vector>
#include "nodes/node.h"

namespace repobuild {

class GenShNode : public Node {
 public:
  GenShNode(const TargetInfo& t,
            const Input& i)
      : Node(t, i) {
  }
  virtual ~GenShNode() {}
  virtual std::string Name() const { return "gen_sh"; }
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void WriteMakeClean(std::string* out) const;
  virtual void WriteMakefile(const std::vector<const Node*>& all_deps,
                             std::string* out) const;

  // Alternative to parse
  void Set(const std::string& build_cmd,
           const std::string& clean_cmd,
           const std::vector<std::string>& input_files,
           const std::vector<std::string>& outputs);

 protected:
  std::string WriteCommand(const std::string& cmd,
                           const std::string& touchfile) const;

  std::string build_cmd_;
  std::string clean_cmd_;
  std::vector<std::string> input_files_, outputs_;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_GEN_SH_H__
