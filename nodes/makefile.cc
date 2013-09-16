// Copyright 2013
// Author: Christopher Van Arsdale

#include "repobuild/nodes/makefile.h"

using std::string;

namespace repobuild {

void Makefile::StartRule(const string& rule, const string& dependencies) {
  out_.append("\n");
  out_.append(rule);
  out_.append(": ");
  out_.append(dependencies);
  out_.append("\n");
}

void Makefile::FinishRule() {
  out_.append("\n");
}

void Makefile::WriteCommand(const string& command) {
  out_.append("\t");
  if (silent_) {
    out_.append("@");
  }
  out_.append(command);
  out_.append("\n");
}

void Makefile::WriteCommandBestEffort(const string& command) {
  out_.append("\t-");  // - == ignore failures
  if (silent_) {
    out_.append("@");
  }
  out_.append(command);
  out_.append("\n");
}

}  // namespace repobuild
