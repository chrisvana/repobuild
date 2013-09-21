// Copyright 2013
// Author: Christopher Van Arsdale

#include "common/strings/path.h"
#include "common/strings/strutil.h"
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
  for (const StringPiece& str : strings::Split(rule->rule(), " ")) {
    registered_rules_.insert(str.as_string());
  }
  delete rule;
}

Makefile::Rule::Rule(const string& rule,
                     const string& dependencies,
                     bool silent)
    : silent_(silent),
      rule_(rule) {
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

void Makefile::Rule::WriteUserEcho(const string& name,
                                   const string& value) {
  WriteCommand(strings::StringPrintf("echo \"%-11s %s\"",
                                     (name + ":").c_str(),
                                     value.c_str()));
}

void Makefile::Rule::MaybeRemoveSymlink(const std::string& path) {
  WriteCommand("[ -L " + path + " ] && rm -f " + path + " || true");
}

void Makefile::WriteRootSymlinkWithDependency(const string& symlink_file,
                                              const string& source_file,
                                              const string& dependencies) {
  string out_dir = strings::PathDirname(symlink_file);

  // Output link target.
  string link = strings::JoinPath(
      strings::Repeat("../", strings::NumPathComponents(out_dir)),
      source_file);

  // Write symlink.
  Rule* rule = StartRule(symlink_file, strings::JoinWith(" ",
                                                         source_file,
                                                         dependencies));
  if (out_dir != ".") {
    rule->WriteCommand("mkdir -p " + out_dir);
  }
  rule->WriteCommand("ln -f -s " + link + " " + symlink_file);
  FinishRule(rule);
}

void Makefile::GenerateExecFile(const std::string& name,
                                const std::string& file_path,
                                const std::string& value) {
  append("define " + name + "\n");
  append(value);
  append("\nendef\n");
  append("export " + name + "\n");
  Makefile::Rule* rule = StartRule(file_path);
  rule->WriteCommand("echo \"$$" + name + "\" > " + file_path);
  rule->WriteCommand("chmod 0755 " + file_path);
  FinishRule(rule);
}

// static
string Makefile::Escape(const string& input) {
  return strings::ReplaceAll(input, "$", "$$");
}

}  // namespace repobuild
