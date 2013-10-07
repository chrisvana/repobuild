// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_GO_TEST_H__
#define _REPOBUILD_NODES_GO_TEST_H__

#include <string>
#include <vector>
#include "repobuild/nodes/node.h"
#include "repobuild/nodes/go_library.h"

namespace repobuild {

class GoTestNode : public GoLibraryNode {
 public:
  GoTestNode(const TargetInfo& t, const Input& i, DistSource* s) 
      : GoLibraryNode(t, i, s) {
  }
  virtual ~GoTestNode() {}
  virtual bool IncludeInAll() const { return false; }
  virtual bool IncludeInTests() const { return true; }
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void LocalWriteMake(Makefile* out) const;
  virtual void LocalTests(LanguageType lang,
                          std::set<std::string>* targets) const;

 protected:
  void WriteGoTest(Makefile* out) const;

  std::vector<std::string> go_build_args_;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_GO_TEST_H__
