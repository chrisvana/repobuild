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
  virtual void LocalWriteMake(Makefile* out) const;
  virtual void LocalDependencyFiles(LanguageType lang,
                                    ResourceFileSet* files) const;
  virtual void LocalObjectFiles(LanguageType lang,
                                ResourceFileSet* files) const;
  virtual void LocalLinkFlags(LanguageType lang,
                              std::set<std::string>* flags) const;
  virtual void LocalCompileFlags(LanguageType lang,
                                 std::set<std::string>* flags) const;
  virtual void LocalIncludeDirs(LanguageType lang,
                                std::set<std::string>* flags) const;

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
                    const ResourceFileSet& input_files,
                    Makefile* out) const;
  void LocalWriteMakeInternal(bool should_write_target, Makefile* out) const;
  Resource ObjForSource(const Resource& source) const;
  void AddVariable(const std::string& cpp_name,
                   const std::string& c_name,
                   const std::string& gcc_value,
                   const std::string& clang_value);

  std::vector<Resource> sources_;
  std::vector<Resource> headers_;
  std::vector<Resource> objects_;

  std::vector<std::string> cc_include_dirs_;

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
