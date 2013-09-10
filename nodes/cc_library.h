// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_CC_LIBRARY_H__
#define _REPOBUILD_NODES_CC_LIBRARY_H__

#include <string>
#include <set>
#include <vector>
#include "repobuild/nodes/node.h"

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
                             Makefile* out) const;
  virtual void DependencyFiles(std::set<Resource>* files) const;
  virtual void ObjectFiles(ObjectFileSet* files) const;
  virtual void LinkFlags(std::set<std::string>* flags) const;
  virtual void CompileFlags(bool cxx, std::set<std::string>* flags) const;

  // Alterative to Parse()
  void Set(const std::vector<Resource>& sources,
           const std::vector<Resource>& headers,
           const std::vector<Resource>& objects,
           const std::vector<std::string>& cc_compile_args,
           const std::vector<std::string>& header_compile_args);

  // Static preprocessors
  static void WriteMakeHead(const Input& input, Makefile* out);

 protected:
  void Init();
  std::string DefaultCompileFlags(bool cpp_mode) const;
  void WriteCompile(const Resource& source,
                    const std::set<Resource>& input_files,
                    const std::vector<const Node*>& all_deps,
                    Makefile* out) const;
  void WriteMakefileInternal(const std::vector<const Node*>& all_deps,
                             bool should_write_target,
                             Makefile* out) const;
  Resource ObjForSource(const Resource& source) const;
  void AddVariable(const std::string& cpp_name,
                   const std::string& c_name,
                   const std::string& gcc_value,
                   const std::string& clang_value);

  std::vector<Resource> sources_;
  std::vector<Resource> headers_;
  std::vector<Resource> objects_;

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
