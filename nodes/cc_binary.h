// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_CC_BINARY_H__
#define _REPOBUILD_NODES_CC_BINARY_H__

#include <string>
#include <vector>
#include "repobuild/nodes/node.h"
#include "repobuild/nodes/cc_library.h"

namespace repobuild {

class CCBinaryNode : public CCLibraryNode {
 public:
  CCBinaryNode(const TargetInfo& t,
               const Input& i)
      : CCLibraryNode(t, i) {
  }
  virtual ~CCBinaryNode() {}
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void LocalWriteMakeClean(Makefile::Rule* out) const;
  virtual void LocalWriteMake(Makefile* out) const;
  virtual void LocalFinalOutputs(LanguageType lang,
                                 ResourceFileSet* outputs) const;
  virtual void LocalDependencyFiles(LanguageType lang,
                                    ResourceFileSet* files) const;
  virtual void LocalBinaries(LanguageType lang,
                             ResourceFileSet* outputs) const;

 protected:
  // Helper.
  Resource OutBinary() const;
  Resource ObjBinary() const;

  void WriteLink(const Resource& file, Makefile* out) const;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_CC_BINARY_H__
