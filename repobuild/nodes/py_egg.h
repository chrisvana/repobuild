// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_PY_EGG_H__
#define _REPOBUILD_NODES_PY_EGG_H__

#include <string>
#include <vector>
#include "repobuild/nodes/node.h"
#include "repobuild/nodes/py_library.h"

namespace repobuild {

class PyEggNode : public PyLibraryNode {
 public:
  PyEggNode(const TargetInfo& t,
            const Input& i,
            DistSource* s)
      : PyLibraryNode(t, i, s) {
  }
  virtual ~PyEggNode() {}
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void LocalWriteMake(Makefile* out) const;

  static void WriteMakeHead(const Input& input, Makefile* out);

 protected:
  void LocalWriteMakeInternal(bool user_base, Makefile* out) const;
  void GetSources(ResourceFileSet* deps,
                  std::vector<std::string>* modules,
                  std::set<std::string>* sys_deps) const;
  void WriteEggFile(const ResourceFileSet& deps,
                    const std::vector<std::string>& modules,
                    const std::set<std::string>& sys_deps,
                    Makefile* out) const;

  Resource OutEgg() const;

  std::vector<std::string> py_build_args_;
  std::string py_version_, py_default_module_;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_PY_EGG_H__
