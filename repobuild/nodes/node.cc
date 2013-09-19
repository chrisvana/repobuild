// Copyright 2013
// Author: Christopher Van Arsdale

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "common/log/log.h"
#include "common/file/fileutil.h"
#include "common/strings/path.h"
#include "common/strings/strutil.h"
#include "common/strings/varmap.h"
#include "common/util/stl.h"
#include "repobuild/env/input.h"
#include "repobuild/json/json.h"
#include "repobuild/nodes/node.h"
#include "repobuild/nodes/util.h"
#include "repobuild/reader/buildfile.h"

using std::map;
using std::string;
using std::vector;
using std::set;

namespace repobuild {

Node::Node(const TargetInfo& target, const Input& input)
    : target_(target),
      input_(&input),
      strict_file_mode_(true) {
  gen_dir_ = strings::JoinPath(input.genfile_dir(), target.dir());
  src_dir_ = strings::JoinPath(input.source_dir(), target.dir());
  obj_dir_ = strings::JoinPath(input.object_dir(), target.dir());
  package_dir_ = strings::JoinPath(input.pkgfile_dir(), target.dir());

  relative_root_dir_ =
      strings::Repeat("../", strings::NumPathComponents(target.dir()));
  relative_gen_dir_ = strings::JoinPath(
      relative_root_dir_,
      strings::JoinPath(input.genfile_dir(), target.dir()));
  relative_src_dir_ = strings::JoinPath(
      relative_root_dir_,
      strings::JoinPath(input.source_dir(), target.dir()));
  relative_obj_dir_ = strings::JoinPath(
      relative_root_dir_,
      strings::JoinPath(input.object_dir(), target.dir()));
}

Node::~Node() {
  DeleteElements(&owned_subnodes_);
}

void Node::Parse(BuildFile* file, const BuildFileNode& input) {
  // Set up build reader.
  CHECK(input.object().isObject())
      << "Expected object for node " << target().full_path();
  build_reader_.reset(NewBuildReader(input));
  current_reader()->ParseBoolField("strict_file_mode", &strict_file_mode_);
  build_reader_->SetStrictFileMode(strict_file_mode_);

  // Figure out our dependencies.
  vector<string> deps;
  current_reader()->ParseRepeatedString("dependencies", &deps);
  for (int i = 0; i < deps.size(); ++i) {
    dep_targets_.push_back(TargetInfo(deps[i], file->filename()));
  }

  // Parse environment variables.
  current_reader()->ParseKeyValueStrings("env", &env_variables_);
}

void Node::WriteMake(Makefile* out) const {
  WriteVariables(out->mutable_out());
  LocalWriteMake(out);
}

void Node::WriteMakeClean(Makefile::Rule* out) const {
  LocalWriteMakeClean(out);
}

void Node::AddDependencyNode(Node* dependency) {
  dependencies_.push_back(dependency);
}

void Node::AddDependencyTarget(const TargetInfo& other) {
  dep_targets_.push_back(other);
}

void Node::CopyDepenencies(Node* other) {
  for (const TargetInfo& t : other->dep_targets()) {
    dep_targets_.push_back(t);
  }
  for (Node* n : other->dependencies()) {
    dependencies_.push_back(n);
  }
}

TargetInfo Node::GetNextTargetName(BuildFile* file) const {
  return target().GetParallelTarget(file->NextName(target().local_path()));
}

void Node::ExtractSubnodes(std::vector<Node*>* nodes) {
  for (Node* n : subnodes_) {
    nodes->push_back(n);
    n->ExtractSubnodes(nodes);
  }
  owned_subnodes_.clear();
}

void Node::AddSubNode(Node* node) {
  AddDependencyTarget(node->target());
  subnodes_.push_back(node);
  owned_subnodes_.push_back(node);
}

BuildFileNodeReader* Node::NewBuildReader(const BuildFileNode& node) const {
  BuildFileNodeReader* reader = new BuildFileNodeReader(node);
  reader->SetReplaceVariable(false, "GEN_DIR", GenDir());
  reader->SetReplaceVariable(true, "GEN_DIR", RelativeGenDir());
  reader->SetReplaceVariable(false, "OBJ_DIR", ObjectDir());
  reader->SetReplaceVariable(true, "OBJ_DIR", RelativeObjectDir());
  reader->SetReplaceVariable(false, "SRC_DIR", target().dir());
  reader->SetReplaceVariable(true, "SRC_DIR", target().dir());
  reader->AddFileAbsPrefix(input().genfile_dir());
  reader->AddFileAbsPrefix(input().source_dir());
  reader->AddFileAbsPrefix(input().object_dir());
  reader->SetStrictFileMode(strict_file_mode_);
  reader->SetErrorPath(target().full_path());
  reader->SetFilePath(target().dir());
  return reader;
}

void Node::CollectAllDependencies(DependencyCollectionType type,
                                  LanguageType lang,
                                  std::vector<Node*>* all_deps) const {
  std::set<Node*> all_deps_set(all_deps->begin(), all_deps->end());
  CollectAllDependencies(type, lang, &all_deps_set, all_deps);
}

void Node::CollectAllDependencies(DependencyCollectionType type,
                                  LanguageType lang,
                                  set<Node*>* all_deps_set,
                                  vector<Node*>* all_deps) const {
  // NB: Order matters here. Anything in the vector will have all of its
  // dependencies listed ahead of it.
  for (Node* node : dependencies_) {
    if (IncludeChildDependency(type, lang, node) &&
        all_deps_set->insert(node).second) {
      if (node->IncludeDependencies(type, lang)) {
        node->CollectAllDependencies(type, lang, all_deps_set, all_deps);
      }
      all_deps->push_back(node);
    }
  }
}

void Node::InputEnvVariables(LanguageType lang,
                             map<string, string>* env) const {
  vector<Node*> all_deps;
  CollectAllDependencies(ENV_VARIABLES, lang, &all_deps);
  for (Node* node : all_deps) {
    node->LocalEnvVariables(lang, env);
  }
}

void Node::LocalEnvVariables(LanguageType lang,
                             map<string, string>* env) const {
  for (const auto& it : env_variables_) {
    (*env)[it.first] = it.second;
  }
}

void Node::InputDependencyFiles(LanguageType lang,
                                ResourceFileSet* files) const {
  vector<Node*> all_deps;
  CollectAllDependencies(DEPENDENCY_FILES, lang, &all_deps);
  for (Node* node : all_deps) {
    node->LocalDependencyFiles(lang, files);
    node->LocalBinaries(lang, files);
  }
}

void Node::InputObjectFiles(LanguageType lang, ResourceFileSet* files) const {
  vector<Node*> all_deps;
  CollectAllDependencies(OBJECT_FILES, lang, &all_deps);
  for (Node* node : all_deps) {
    node->LocalObjectFiles(lang, files);
  }
}

void Node::InputFinalOutputs(LanguageType lang,
                             ResourceFileSet* outputs) const {
  vector<Node*> all_deps;
  CollectAllDependencies(FINAL_OUTPUTS, lang, &all_deps);
  for (Node* node : all_deps) {
    node->LocalFinalOutputs(lang, outputs);
  }
}

void Node::InputBinaries(LanguageType lang, ResourceFileSet* outputs) const {
  vector<Node*> all_deps;
  CollectAllDependencies(BINARIES, lang, &all_deps);
  for (Node* node : all_deps) {
    node->LocalBinaries(lang, outputs);
  }
}

void Node::InputLinkFlags(LanguageType lang, set<string>* flags) const {
  vector<Node*> all_deps;
  CollectAllDependencies(LINK_FLAGS, lang, &all_deps);
  for (Node* node : all_deps) {
    node->LocalLinkFlags(lang, flags);
  }
}

void Node::InputCompileFlags(LanguageType lang, set<string>* flags) const {
  vector<Node*> all_deps;
  CollectAllDependencies(COMPILE_FLAGS, lang, &all_deps);
  for (Node* node : all_deps) {
    node->LocalCompileFlags(lang, flags);
  }
}

void Node::InputIncludeDirs(LanguageType lang, set<string>* dirs) const {
  vector<Node*> all_deps;
  CollectAllDependencies(INCLUDE_DIRS, lang, &all_deps);
  for (Node* node : all_deps) {
    node->LocalIncludeDirs(lang, dirs);
  }
  dirs->insert(input().root_dir());
  dirs->insert(input().source_dir());
  dirs->insert(input().genfile_dir());
}

void Node::EnvVariables(LanguageType lang, map<string, string>* env) const {
  InputEnvVariables(lang, env);
  LocalEnvVariables(lang, env);
}

void Node::DependencyFiles(LanguageType lang, ResourceFileSet* files) const {
  InputDependencyFiles(lang, files);
  LocalDependencyFiles(lang, files);
}

void Node::ObjectFiles(LanguageType lang, ResourceFileSet* files) const {
  InputObjectFiles(lang, files);
  LocalObjectFiles(lang, files);
}

void Node::FinalOutputs(LanguageType lang, ResourceFileSet* outputs) const {
  InputFinalOutputs(lang, outputs);
  LocalFinalOutputs(lang, outputs);
}

void Node::Binaries(LanguageType lang, ResourceFileSet* outputs) const {
  InputBinaries(lang, outputs);
  LocalBinaries(lang, outputs);
}

void Node::LinkFlags(LanguageType lang, set<string>* flags) const {
  InputLinkFlags(lang, flags);
  LocalLinkFlags(lang, flags);
}

void Node::CompileFlags(LanguageType lang, set<string>* flags) const {
  InputCompileFlags(lang, flags);
  LocalCompileFlags(lang, flags);
}

void Node::IncludeDirs(LanguageType lang, set<string>* dirs) const {
  InputIncludeDirs(lang, dirs);
  LocalIncludeDirs(lang, dirs);
}

void Node::WriteBaseUserTarget(Makefile* out) const {
  ResourceFileSet empty;
  WriteBaseUserTarget(empty, out);
}

void Node::WriteBaseUserTarget(const ResourceFileSet& deps,
                               Makefile* out) const {
  if (out->seen_rule(target().make_path())) {
    return;
  }

  out->append(target().make_path());
  out->append(":");
  for (const Resource& dep : deps.files()) {
    out->append(" ");
    out->append(dep.path());
  }
  for (const TargetInfo& dep : dep_targets()) {
    if (dep.make_path() != target().make_path()) {
      out->append(" ");
      out->append(dep.make_path());
    }
  }
  out->append("\n\n.PHONY: ");
  out->append(target().make_path());
  out->append("\n\n");
}

Node::MakeVariable::MakeVariable(const std::string& name)
    : name_(name) {
}

Node::MakeVariable::~MakeVariable() {
}

const std::string& Node::MakeVariable::name() const { return name_; }

std::string Node::MakeVariable::ref_name() const {
  return name_.empty() ? "" : "$(" + name_ + ")";
}

void Node::MakeVariable::SetValue(const std::string& value) {
  SetCondition("", value, "");
}

void Node::MakeVariable::SetCondition(const std::string& condition,
                                const std::string& if_val,
                                const std::string& else_val) {
  conditions_[condition] = std::make_pair(if_val, else_val);
}

void Node::MakeVariable::WriteMake(string* out) const {
  if (name_.empty()) {
    return;
  }
  out->append(name_);
  out->append(" := ");
  auto it = conditions_.find("");
  if (it != conditions_.end()) {
    out->append(it->second.first);
  }
  out->append("\n");
  for (const auto& it : conditions_) {
    if (it.first.empty()) {
      continue;
    }

    out->append("ifeq ($(");
    out->append(it.first);
    out->append("),1)\n\t");
    out->append(name_);
    out->append(" := ");
    out->append(it.second.first);
    if (!it.second.second.empty()) {
      out->append("\nelse\n\t");
      out->append(name_);
      out->append(" := ");
      out->append(it.second.second);
    }
    out->append("\nendif\n");
  }
  out->append("\n");
}

Resource Node::Touchfile(const string& suffix) const {
  return Resource::FromLocalPath(
      strings::JoinPath(input().object_dir(), target().dir()),
      "." + target().local_path() + suffix + ".dummy");
}

void Node::WriteVariables(std::string* out) const {
  for (auto const& it : make_variables_) {
    it.second->WriteMake(out);
  }
}

bool Node::HasVariable(const std::string& name) const {
  return make_variables_.find(name) != make_variables_.end();
}

const Node::MakeVariable& Node::GetVariable(const std::string& name) const {
  static MakeVariable kEmpty("");
  const auto& it = make_variables_.find(name);
  if (it == make_variables_.end()) {
    return kEmpty;
  }
  return *it->second;
}

Node::MakeVariable* Node::MutableVariable(const std::string& name) {
  MakeVariable** var = &(make_variables_[name]);
  if (*var == NULL) {
    *var = new MakeVariable(name + "." + target().make_path());
  }
  return *var;
}

string Node::StripSpecialDirs(const string& path) const {
  return NodeUtil::StripSpecialDirs(input(), path);
}

}  // namespace repobuild
