// Copyright 2013
// Author: Christopher Van Arsdale

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

void Node::WriteMakeClean(Makefile* out) const {
  LocalWriteMakeClean(out);
}

void Node::AddDependencyTarget(const TargetInfo& other) {
  dep_targets_.push_back(other);
}

void Node::AddDependencyNode(Node* dependency) {
  dependencies_.push_back(dependency);
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

void Node::CollectAllDependencies(const DependencyCollectionType& type,
                                  set<Node*>* all_deps_set,
                                  vector<Node*>* all_deps) const {
  // NB: Order matters here. Anything in the vector will have all of its
  // dependencies listed ahead of it.
  if (IncludeDependencies(type)) {
    for (Node* node : dependencies_) {
      if (all_deps_set->insert(node).second) {
        node->CollectAllDependencies(type, all_deps_set, all_deps);
        all_deps->push_back(node);
      }
    }
  }
}

void Node::InputEnvVariables(map<string, string>* env) const {
  vector<Node*> all_deps;
  CollectAllDependencies(ENV_VARIABLES, &all_deps);
  for (Node* node : all_deps) {
    node->LocalEnvVariables(env);
  }
}

void Node::LocalEnvVariables(map<string, string>* env) const {
  for (const auto& it : env_variables_) {
    (*env)[it.first] = it.second;
  }
}

void Node::InputDependencyFiles(ResourceFileSet* files) const {
  vector<Node*> all_deps;
  CollectAllDependencies(DEPENDENCY_FILES, &all_deps);
  for (Node* node : all_deps) {
    node->LocalDependencyFiles(files);
  }
}

void Node::InputObjectFiles(ResourceFileSet* files) const {
  vector<Node*> all_deps;
  CollectAllDependencies(OBJECT_FILES, &all_deps);
  for (Node* node : all_deps) {
    node->LocalObjectFiles(files);
  }
}

void Node::InputFinalOutputs(ResourceFileSet* outputs) const {
  vector<Node*> all_deps;
  CollectAllDependencies(FINAL_OUTPUTS, &all_deps);
  for (Node* node : all_deps) {
    node->LocalFinalOutputs(outputs);
  }
}

void Node::InputLinkFlags(set<string>* flags) const {
  vector<Node*> all_deps;
  CollectAllDependencies(LINK_FLAGS, &all_deps);
  for (Node* node : all_deps) {
    node->LocalLinkFlags(flags);
  }
}

void Node::InputCompileFlags(bool cxx, set<string>* flags) const {
  vector<Node*> all_deps;
  CollectAllDependencies(COMPILE_FLAGS, &all_deps);
  for (Node* node : all_deps) {
    node->LocalCompileFlags(cxx, flags);
  }
}

void Node::InputIncludeDirs(set<string>* dirs) const {
  vector<Node*> all_deps;
  CollectAllDependencies(INCLUDE_DIRS, &all_deps);
  for (Node* node : all_deps) {
    node->LocalIncludeDirs(dirs);
  }
}

void Node::EnvVariables(map<string, string>* env) const {
  InputEnvVariables(env);
  LocalEnvVariables(env);
}

void Node::DependencyFiles(ResourceFileSet* files) const {
  InputDependencyFiles(files);
  LocalDependencyFiles(files);
}

void Node::ObjectFiles(ResourceFileSet* files) const {
  InputObjectFiles(files);
  LocalObjectFiles(files);
}

void Node::FinalOutputs(ResourceFileSet* outputs) const {
  InputFinalOutputs(outputs);
  LocalFinalOutputs(outputs);
}

void Node::LinkFlags(set<string>* flags) const {
  InputLinkFlags(flags);
  LocalLinkFlags(flags);
}

void Node::CompileFlags(bool cxx, set<string>* flags) const {
  InputCompileFlags(cxx, flags);
  LocalCompileFlags(cxx, flags);
}

void Node::IncludeDirs(set<string>* dirs) const {
  InputIncludeDirs(dirs);
  LocalIncludeDirs(dirs);
}

string Node::MakefileEscape(const string& str) const {
  return strings::ReplaceAll(str, "$", "$$");
}

void Node::WriteBaseUserTarget(const ResourceFileSet& deps,
                               Makefile* out) const {
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

void SimpleLibraryNode::LocalDependencyFiles(ResourceFileSet* files) const {
  Node::LocalDependencyFiles(files);
  for (int i = 0; i < sources_.size(); ++i) {
    files->Add(sources_[i]);
  }
}

void SimpleLibraryNode::LocalObjectFiles(ResourceFileSet* files) const {
  Node::LocalObjectFiles(files);
  for (int i = 0; i < sources_.size(); ++i) {
    files->Add(sources_[i]);
  }
}

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
