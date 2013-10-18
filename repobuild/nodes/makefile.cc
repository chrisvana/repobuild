// Copyright 2013
// Author: Christopher Van Arsdale

#include <string>
#include <set>
#include "common/strings/path.h"
#include "common/strings/strutil.h"
#include "repobuild/nodes/makefile.h"

using std::set;
using std::string;

namespace repobuild {
namespace {
const char kPrereqRuleFile[] = ".dummy.prereqs";
}  // anonymous namespace

Makefile::Rule* Makefile::StartRawRule(const string& rule,
                                       const string& dependencies) {
  return new Rule(rule, dependencies, silent_);
}

Makefile::Rule* Makefile::StartPrereqRule(const string& rule,
                                          const string& dependencies) {
  prereq_rules_.insert(rule);
  return StartRawRule(rule, dependencies);
}

Makefile::Rule* Makefile::StartRule(const string& rule,
                                    const string& dependencies) {
  return StartRawRule(rule, strings::JoinWith(" ", dependencies,
                                              GetPrereqFile()));
}

void Makefile::FinishRule(Makefile::Rule* rule) {
  out_.append("\n");
  out_.append(rule->rule() + ": " + rule->dependencies() + "\n");
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
      rule_(rule),
      dependencies_(dependencies) {
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

void Makefile::Rule::AddDependency(const string& dep) {
  if (!dependencies_.empty()) {
    dependencies_ += " ";
  }
  dependencies_ += dep;
}

void Makefile::Rule::MaybeRemoveSymlink(const string& path) {
  WriteCommand("[ -L " + path + " ] && rm -f " + path + " || true");
}

void Makefile::WriteRootSymlinkWithDependency(const string& symlink_file,
                                              const string& source_file,
                                              const string& dependencies) {
  string out_dir = strings::PathDirname(symlink_file);

  // Output link target.
  string link = strings::GetRelativePath(out_dir, source_file);

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

void Makefile::GenerateExecFile(const string& name,
                                const string& file_path,
                                const string& value) {
  append("define " + name + "\n");
  append(strings::Base64Encode(value));
  append("\nendef\n");
  append("export " + name + "\n");
  Makefile::Rule* rule = StartRawRule(file_path, "");
  rule->WriteCommand("mkdir -p " + strings::PathDirname(file_path));
  rule->WriteCommand("echo \"$$" + name + "\" | base64 --decode > "
                     + file_path);
  rule->WriteCommand("chmod 0755 " + file_path);
  FinishRule(rule);
}

void Makefile::FinishMakefile() {
  Rule* rule = StartRawRule(GetPrereqFile(),
                            strings::JoinAll(prereq_rules_, " "));
  rule->WriteCommand("mkdir -p " + scratch_dir_);
  rule->WriteCommand("touch " + GetPrereqFile());
  FinishRule(rule);

  FinishRule(StartRawRule("prereqs", GetPrereqFile()));
  append(".PHONY: prereqs\n\n");
}

string Makefile::GetPrereqFile() const {
  return scratch_dir_ + "/" + kPrereqRuleFile;
}

// static
string Makefile::Escape(const string& input) {
  return strings::ReplaceAll(input, "$", "$$");
}

}  // namespace repobuild
