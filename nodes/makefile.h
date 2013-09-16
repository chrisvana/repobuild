// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_MAKEFILE_H__
#define _REPOBUILD_NODES_MAKEFILE_H__

#include <string>
#include "common/strings/strutil.h"

namespace repobuild {

class Makefile {
 public:
  Makefile() :silent_(true) {}
  ~Makefile() {}

  // Options
  void SetSilent(bool silent) { silent_ = silent; }

  // Rules
  // TODO(cvanarsdale): Return a pointer to a MakefileRule.
  void StartRule(const std::string& rule) { StartRule(rule, ""); }
  void StartRule(const std::string& rule, const std::string& dependencies);
  void FinishRule();
  void WriteRule(const std::string& rule, const std::string& deps) {
    StartRule(rule, deps);
    FinishRule();
  }

  // Commands for rules.
  void WriteCommand(const std::string& command);
  void WriteCommandBestEffort(const std::string& command);

  std::string* mutable_out() { return &out_; }
  const std::string& out() const { return out_; }

  template <typename T>
  void append(const T& t) {
    out_.append(strings::StringPrint(t));
  }

 private:
  bool silent_;
  std::string out_;
};

}  // namespace repobuild

#endif  // _REPOBUILD_NODES_MAKEFILE_H__
