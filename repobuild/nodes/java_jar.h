// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_JAVA_JAR_H__
#define _REPOBUILD_NODES_JAVA_JAR_H__

#include <string>
#include <vector>
#include "repobuild/nodes/node.h"
#include "repobuild/nodes/java_library.h"

namespace repobuild {

class JavaJarNode : public JavaLibraryNode {
 public:
  JavaJarNode(const TargetInfo& t,
              const Input& i,
              DistSource* s)
      : JavaLibraryNode(t, i, s) {
  }
  virtual ~JavaJarNode() {}
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void LocalWriteMake(Makefile* out) const;

 protected:
  // Helper.
  Resource JarName() const;
  void LocalWriteMakeInternal(bool write_base, Makefile* out) const;
  void WriteJar(const Resource& file, Makefile* out) const;

  std::vector<std::string> java_manifest_;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_JAVA_JAR_H__
