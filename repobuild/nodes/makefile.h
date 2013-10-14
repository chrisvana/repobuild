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
  explicit Makefile(const std::string& root_dir,
                    const std::string& scratch_dir) 
      : silent_(true),
        root_dir_(root_dir),
        scratch_dir_(scratch_dir) {
  }
  ~Makefile() {}

  // Options
  const std::string& root_dir() const { return root_dir_; }
  const std::string& scratch_dir() const { return scratch_dir_; }
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

    void AddDependency(const std::string& dep);

    // Raw access.
    std::string* mutable_out() { return &out_; }
    const std::string& out() const { return out_; }
    const std::string& rule() const { return rule_; }
    const std::string& dependencies() const { return dependencies_; }

   private:
    bool silent_;
    std::string rule_;
    std::string dependencies_;
    std::string out_;
  };

  // Rules
  Rule* StartRule(const std::string& rule) { return StartRule(rule, ""); }
  Rule* StartRule(const std::string& rule, const std::string& dependencies);
  void FinishRule(Rule* rule);
  void WriteRule(const std::string& rule, const std::string& deps) {
    FinishRule(StartRule(rule, deps));
  }
  Rule* StartPrereqRule(const std::string& rule,
                        const std::string& dependencies);
  Rule* StartRawRule(const std::string& rule,
                     const std::string& dependencies);

  bool seen_rule(const std::string& rule) const {
    return registered_rules_.find(rule) != registered_rules_.end();
  }

  void FinishMakefile();

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
  void GenerateExecFile(const std::string& name,
                        const std::string& file_path,
                        const std::string& value);

  static std::string Escape(const std::string& input);

 private:
  std::string GetPrereqFile() const;

  bool silent_;
  std::string root_dir_, scratch_dir_;
  std::string out_;
  std::set<std::string> registered_rules_;
  std::set<std::string> prereq_rules_;
};

}  // namespace repobuild

#endif  // _REPOBUILD_NODES_MAKEFILE_H__
