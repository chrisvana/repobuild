// Copyright 2013
// Author: Christopher Van Arsdale

#include "repobuild/nodes/makefile.h"

using std::string;

namespace repobuild {

Makefile::Rule* Makefile::StartRule(const string& rule,
                                    const string& dependencies) {
  return new Rule(rule, dependencies, silent_);
}

void Makefile::FinishRule(Makefile::Rule* rule) {
  out_.append("\n");
  out_.append(rule->out());  
  out_.append("\n");
  delete rule;
}

Makefile::Rule::Rule(const string& rule,
                     const string& dependencies,
                     bool silent)
    : silent_(silent) {
  out_.append(rule);
  out_.append(": ");
  out_.append(dependencies);
  out_.append("\n");
}

void Makefile::Rule::WriteCommand(const string& command) {
  out_.append("\t");
  if (silent_) {
    out_.append("@");
  }
  out_.append(command);
  out_.append("\n");
}

void Makefile::Rule::WriteCommandBestEffort(const string& command) {
  out_.append("\t-");  // - == ignore failures
  if (silent_) {
    out_.append("@");
  }
  out_.append(command);
  out_.append("\n");
}

}  // namespace repobuild
