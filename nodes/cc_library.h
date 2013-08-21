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
  virtual void CompileFlags(bool cxx, std::set<std::string>* flags) const;

  // Alterative to Parse()
  void Set(const std::vector<std::string>& sources,
           const std::vector<std::string>& headers,
           const std::vector<std::string>& objects,
           const std::vector<std::string>& cc_compile_args,
           const std::vector<std::string>& header_compile_args);

  // Static preprocessors
  static void WriteMakeHead(const Input& input, std::string* out);

 protected:
  void Init();
  std::string DefaultCompileFlags(bool cpp_mode) const;
  void WriteCompile(const std::string& source,
                    const std::set<std::string>& input_files,
                    const std::vector<const Node*>& all_deps,
                    std::string* out) const;
  void WriteMakefileInternal(const std::vector<const Node*>& all_deps,
                             bool should_write_target,
                             std::string* out) const;
  std::string ObjForSource(const std::string& source) const;
  void AddVariable(const std::string& cpp_name,
                   const std::string& c_name,
                   const std::string& gcc_value,
                   const std::string& clang_value);

  std::vector<std::string> sources_;
  std::vector<std::string> headers_;
  std::vector<std::string> objects_;

  std::vector<std::string> cc_compile_args_;
  std::vector<std::string> header_compile_args_;
  std::vector<std::string> cc_linker_args_;

  std::vector<std::string> gcc_cc_compile_args_;
  std::vector<std::string> gcc_header_compile_args_;
  std::vector<std::string> gcc_cc_linker_args_;

  std::vector<std::string> clang_cc_compile_args_;
  std::vector<std::string> clang_header_compile_args_;
  std::vector<std::string> clang_cc_linker_args_;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_CC_LIBRARY_H__
