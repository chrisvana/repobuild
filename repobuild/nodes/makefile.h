// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_MAKEFILE_H__
#define _REPOBUILD_NODES_MAKEFILE_H__

#include <set>
#include <string>
#include "common/strings/strutil.h"

namespace repobuild {

class Makefile {
 public:
  Makefile() :silent_(true) {}
  ~Makefile() {}

  // Options
  void SetSilent(bool silent) { silent_ = silent; }

  class Rule {
   public:
    Rule(const std::string& rule, const std::string& dependencies, bool silent);
    ~Rule() {}

    // Adding commands to our rule.
    void WriteCommand(const std::string& command);
    void WriteCommandBestEffort(const std::string& command);
    void WriteUserEcho(const std::string& name,
                       const std::string& value);  // prints "name: value".
    void MaybeRemoveSymlink(const std::string& path);

    // Raw access.
    std::string* mutable_out() { return &out_; }
    const std::string& out() const { return out_; }
    const std::string& rule() const { return rule_; }

   private:
    bool silent_;
    std::string rule_;
    std::string out_;
  };

  // Rules
  Rule* StartRule(const std::string& rule) { return StartRule(rule, ""); }
  Rule* StartRule(const std::string& rule, const std::string& dependencies);
  void FinishRule(Rule* rule);
  void WriteRule(const std::string& rule, const std::string& deps) {
    FinishRule(StartRule(rule, deps));
  }
  bool seen_rule(const std::string& rule) const {
    return registered_rules_.find(rule) != registered_rules_.end();
  }

  // Full access.
  std::string* mutable_out() { return &out_; }
  const std::string& out() const { return out_; }
  template <typename T> void append(const T& t) {
    out_.append(strings::StringPrint(t));
  }

  // Symlink shortcuts.
  // Each file path must have the same root.
  void WriteRootSymlink(const std::string& symlink_file,
                        const std::string& source_file) {
    WriteRootSymlinkWithDependency(symlink_file, source_file, "");
  }
  void WriteRootSymlinkWithDependency(const std::string& symlink_file,
                                      const std::string& source_file,
                                      const std::string& depenencies);

  // Generated files.
  void GenerateFile(const std::string& name,
                    const std::string& value,
                    const std::string& file_path);

  static std::string Escape(const std::string& input);

 private:
  bool silent_;
  std::string out_;
  std::set<std::string> registered_rules_;
};

}  // namespace repobuild

#endif  // _REPOBUILD_NODES_MAKEFILE_H__
