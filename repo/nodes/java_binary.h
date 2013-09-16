// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_JAVA_BINARY_H__
#define _REPOBUILD_NODES_JAVA_BINARY_H__

#include <string>
#include <vector>
#include "repobuild/nodes/node.h"
#include "repobuild/nodes/java_library.h"

namespace repobuild {

class JavaBinaryNode : public JavaLibraryNode {
 public:
  JavaBinaryNode(const TargetInfo& t,
                 const Input& i)
      : JavaLibraryNode(t, i) {
  }
  virtual ~JavaBinaryNode() {}
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void LocalWriteMakeClean(Makefile::Rule* out) const;
  virtual void LocalWriteMake(Makefile* out) const;
  virtual void LocalDependencyFiles(LanguageType lang,
                                    ResourceFileSet* files) const;
  virtual void LocalFinalOutputs(LanguageType lang,
                                 ResourceFileSet* outputs) const;
  virtual void LocalBinaries(LanguageType lang,
                             ResourceFileSet* outputs) const;

 protected:
  // Helper.
  Resource JarName() const;
  Resource OutJarName() const;

  void WriteJar(const Resource& file, Makefile* out) const;

  std::vector<std::string> java_manifest_;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_JAVA_BINARY_H__
