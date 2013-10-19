// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_PY_BINARY_H__
#define _REPOBUILD_NODES_PY_BINARY_H__

#include <string>
#include <vector>
#include "repobuild/nodes/node.h"
#include "repobuild/nodes/py_egg.h"

namespace repobuild {

class PyBinaryNode : public PyEggNode {
 public:
  PyBinaryNode(const TargetInfo& t,
               const Input& i,
               DistSource* s)
      : PyEggNode(t, i, s) {
  }
  virtual ~PyBinaryNode() {}
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void LocalWriteMake(Makefile* out) const;
  virtual void LocalBinaries(LanguageType lang,
                             ResourceFileSet* outputs) const;
  virtual bool ShouldInclude(DependencyCollectionType type,
                             LanguageType lang) const;

 protected:
  Resource BinScript() const;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_PY_BINARY_H__
