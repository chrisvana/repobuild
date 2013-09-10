// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_JAVA_LIBRARY_H__
#define _REPOBUILD_NODES_JAVA_LIBRARY_H__

#include <set>
#include <string>
#include <vector>
#include "repobuild/nodes/node.h"

namespace repobuild {

class JavaLibraryNode : public Node {
 public:
  JavaLibraryNode(const TargetInfo& t,
                const Input& i)
      : Node(t, i) {
  }
  virtual ~JavaLibraryNode() {}
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void WriteMakefile(Makefile* out) const {
    WriteMakefileInternal(true, out);
  }
  virtual void ObjectFiles(ObjectFileSet* files) const;
  virtual void LinkFlags(std::set<std::string>* flags) const;
  virtual void CompileFlags(bool cxx, std::set<std::string>* flags) const;

 protected:
  void WriteMakefileInternal(bool write_user_target, Makefile* out) const;
  void WriteCompile(const Resource& source,
                    const std::set<Resource>& input_files,
                    Makefile* out) const;
  Resource ClassFile(const Resource& source) const;

  std::vector<Resource> sources_;
  std::vector<std::string> java_local_compile_args_;
  std::vector<std::string> java_compile_args_;
  std::vector<std::string> java_jar_args_;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_JAVA_LIBRARY_H__
