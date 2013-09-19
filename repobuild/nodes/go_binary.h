// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_GO_BINARY_H__
#define _REPOBUILD_NODES_GO_BINARY_H__

#include <string>
#include <vector>
#include "repobuild/nodes/node.h"
#include "repobuild/nodes/go_library.h"

namespace repobuild {

class GoBinaryNode : public GoLibraryNode {
 public:
  GoBinaryNode(const TargetInfo& t,
               const Input& i)
      : GoLibraryNode(t, i) {
  }
  virtual ~GoBinaryNode() {}
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void LocalWriteMake(Makefile* out) const;
  virtual void LocalBinaries(LanguageType lang,
                             ResourceFileSet* outputs) const;

 protected:
  void WriteGoBinary(const Resource& bin, Makefile* out) const;
  Resource Binary() const;

  std::vector<std::string> go_build_args_;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_GO_BINARY_H__
