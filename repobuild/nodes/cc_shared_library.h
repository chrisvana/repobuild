// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_CC_SHARED_LIBRARY_H__
#define _REPOBUILD_NODES_CC_SHARED_LIBRARY_H__

#include <string>
#include <vector>
#include "repobuild/env/resource.h"
#include "repobuild/nodes/node.h"
#include "repobuild/nodes/cc_library.h"

namespace repobuild {

class CCSharedLibraryNode : public CCLibraryNode {
 public:
  CCSharedLibraryNode(const TargetInfo& t,
                      const Input& i,
                      DistSource* s)
      : CCLibraryNode(t, i, s) {
  }
  virtual ~CCSharedLibraryNode() {}
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void LocalWriteMake(Makefile* out) const;
  virtual void ObjectFiles(LanguageType lang,
                           ResourceFileSet* files) const;

  static void WriteMakeHead(const Input& input, Makefile* out);

 protected:
  Resource OutLinkedObj() const;
  void WriteLink(Makefile* out) const;

  std::string major_version_, minor_version_, release_version_;
  Resource exported_symbols_;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_CC_SHARED_LIBRARY_H__
