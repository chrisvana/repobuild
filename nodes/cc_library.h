// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_CC_LIBRARY_H__
#define _REPOBUILD_NODES_CC_LIBRARY_H__

#include <string>
#include <set>
#include <vector>
#include "nodes/node.h"

namespace repobuild {

class CCLibraryNode : public Node {
 public:
  CCLibraryNode(const TargetInfo& t,
                const Input& i)
      : Node(t, i) {
  }
  virtual ~CCLibraryNode() {}
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void WriteMakefile(const std::vector<const Node*>& all_deps,
                             std::string* out) const;
  virtual void DependencyFiles(std::vector<std::string>* files) const;
  virtual void ObjectFiles(std::vector<std::string>* files) const;
  virtual void LinkFlags(std::set<std::string>* flags) const;
  virtual void CompileFlags(std::set<std::string>* flags) const;

  // Alterative to Parse()
  void Set(const std::vector<std::string>& sources,
           const std::vector<std::string>& headers,
           const std::vector<std::string>& objects,
           const std::vector<std::string>& cc_compile_args,
           const std::vector<std::string>& header_compile_args);

 protected:
  std::string DefaultCompileFlags() const;
  void WriteCompile(const std::string& source,
                    const std::set<std::string>& input_files,
                    const std::vector<const Node*>& all_deps,
                    std::string* out) const;

  std::vector<std::string> sources_;
  std::vector<std::string> headers_;
  std::vector<std::string> objects_;
  std::vector<std::string> cc_compile_args_;
  std::vector<std::string> header_compile_args_;
  std::vector<std::string> cc_linker_args_;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_CC_LIBRARY_H__
