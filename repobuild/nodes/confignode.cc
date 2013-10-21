// Copyright 2013
// Author: Christopher Van Arsdale

#include <memory>
#include <set>
#include <string>
#include <vector>
#include "common/log/log.h"
#include "common/strings/path.h"
#include "repobuild/env/input.h"
#include "repobuild/env/resource.h"
#include "repobuild/nodes/confignode.h"
#include "repobuild/nodes/makefile.h"
#include "repobuild/nodes/util.h"
#include "repobuild/reader/buildfile.h"

using std::set;
using std::string;
using std::vector;

namespace repobuild {
namespace {
class ConfigRewriter : public BuildFile::BuildDependencyRewriter {
 public:
  explicit ConfigRewriter(ComponentHelper* helper)
      : helper_(helper) {
  }
  virtual ~ConfigRewriter() {}
  virtual bool RewriteDependency(TargetInfo* target) {
    return helper_->RewriteDependency(target);
  }

 private:
  std::unique_ptr<ComponentHelper> helper_;
};
}

ConfigNode::ConfigNode(const TargetInfo& target,
                       const Input& input,
                       DistSource* source)
    : Node(target, input, source) {
}

ConfigNode::~ConfigNode() {}

void ConfigNode::Parse(BuildFile* file, const BuildFileNode& input) {
  Node::Parse(file, input);
  file->AddBaseDependency(target().full_path());

  // Figure out our plugins
  vector<string> plugins;
  current_reader()->ParseRepeatedString("plugins", &plugins);
  for (const string& plugin : plugins) {
    AddPreParse(TargetInfo(plugin));
  }

  // Initialize ComponentHelper.
  string component_src;
  if (!current_reader()->ParseStringField("component", &component_src)) {
    LOG(FATAL) << "Could not parse \"component\" in "
               << target().dir() << " config node.";
  }
  if (!component_src.empty()) {
    string component_root =
        current_reader()->ParseSingleDirectory(false, "component_root");
    if (component_root.empty()) {
      component_root = target().dir();
    }
    component_.reset(new ComponentHelper(component_src, component_root));
  }

  source_dummy_file_ =
      Resource::FromRootPath(DummyFile(SourceDir("")));
  gendir_dummy_file_ =
      Resource::FromRootPath(DummyFile(SourceDir(Node::input().genfile_dir())));
  pkgfile_dummy_file_ =
      Resource::FromRootPath(DummyFile(SourceDir(Node::input().pkgfile_dir())));

  if (component_.get() != NULL) {
    file->AddDependencyRewriter(new ConfigRewriter(component_->Clone()));
  }
}

void ConfigNode::LocalWriteMake(Makefile* out) const {
  if (component_.get() == NULL) {
    return;
  }

  const string& actual_dir = component_->base_dir();

  // 3 Rules:
  // 1) Our .gen-src directory
  // 2) Our .gen-src/.gen-pkg directory (HACK).
  // 3) .gen-src/.gen-files directory (HACK).
  // 4) User target that generates all 3 above.
  ResourceFileSet dirs;

  {  // (1) .gen-src symlink
    Resource dir = Resource::FromRootPath(source_dummy_file_.dirname());
    dirs.Add(dir);
    AddSymlink(dir.path(), actual_dir, out);
  }

  {  // (2) .gen-src/.gen-pkg symlink
    Resource dir = Resource::FromRootPath(pkgfile_dummy_file_.dirname());
    dirs.Add(dir);
    AddSymlink(dir.path(),
               strings::JoinPath(input().pkgfile_dir(), actual_dir),
               out);
  }

  {  // (3) .gen-src/.gen-files symlink
    Resource dir = Resource::FromRootPath(gendir_dummy_file_.dirname());
    dirs.Add(dir);
    AddSymlink(dir.path(),
               strings::JoinPath(input().genfile_dir(), actual_dir),
               out);
  }

  // (4) User target.
  WriteBaseUserTarget(dirs, out);
}

void ConfigNode::AddSymlink(const string& dir,
                            const string& source,
                            Makefile* out) const {
  // Output link target.
  string link = strings::JoinPath(
      strings::Repeat("../",
                      strings::NumPathComponents(strings::PathDirname(dir))),
      source);

  // Write symlink.
  Makefile::Rule* rule = out->StartRule(dir);
  rule->WriteCommand(strings::Join(
      "mkdir -p ", strings::PathDirname(dir), "; ",
      "[ -d ", source, " ] || mkdir -p ", source, "; ",
      "ln -f -s ", link, " ", dir));
  out->FinishRule(rule);

  // Dummy file (to avoid directory timestamp causing everything to rebuild).
  // .gen-src/repobuild/.dummy: .gen-src/repobuild
  //   [ -f .gen-src/repobuild/.dummy ] || touch .gen-src/repobuild/.dummy
  string dummy = DummyFile(dir);
  rule = out->StartRule(dummy, dir);
  rule->WriteCommand(strings::Join("[ -f ", dummy, " ] || touch ", dummy));
  out->FinishRule(rule);
}

void ConfigNode::LocalWriteMakeClean(Makefile::Rule* rule) const {
  if (component_.get() == NULL) {
    return;
  }

  rule->WriteCommand("rm -rf " + source_dummy_file_.path());
  rule->WriteCommand("rm -rf " + gendir_dummy_file_.path());
  rule->WriteCommand("rm -rf " + pkgfile_dummy_file_.path());
}

void ConfigNode::LocalDependencyFiles(LanguageType lang,
                                      ResourceFileSet* files) const {
  if (component_.get() == NULL) {
    return;
  }

  files->Add(source_dummy_file_);
  files->Add(gendir_dummy_file_);
  files->Add(pkgfile_dummy_file_);
}

void ConfigNode::LocalIncludeDirs(LanguageType lang, set<string>* dirs) const {
  if (component_.get() == NULL) {
    return;
  }

  dirs->insert(strings::JoinPath(input().source_dir(),
                                 input().genfile_dir()));
}

bool ConfigNode::PathRewrite(std::string* output_path,
                             std::string* rewrite_root) const {
  if (component_.get() == NULL) {
    return false;
  }

  *output_path = component_->component();
  *rewrite_root = component_->base_dir();
  return true;
}

string ConfigNode::DummyFile(const string& dir) const {
  return strings::JoinPath(dir, ".dummy");
}

string ConfigNode::SourceDir(const string& middle) const {
  const string& component = component_->component();
  if (middle.empty()) {
    return strings::JoinPath(input().source_dir(), component);
  }
  return strings::JoinPath(input().source_dir(),
                           strings::JoinPath(middle, component));
}

}  // namespace repobuild
