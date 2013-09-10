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

void Node::WriteMake(const vector<const Node*>& all_deps,
                     Makefile* out) const {
  WriteVariables(out->mutable_out());
  WriteMakefile(all_deps, out);
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

void Node::EnvVariables(map<string, string>* env) const {
  for (Node* node : dependencies_) {
    node->EnvVariables(env);
  }
  for (const auto& it : env_variables_) {
    (*env)[it.first] = it.second;
  }
}

void Node::DependencyFiles(set<Resource>* files) const {
  for (Node* node : dependencies_) {
    node->DependencyFiles(files);
  }
}

void Node::ObjectFiles(ObjectFileSet* files) const {
  // NB: Order matters for gcc object files (sadly), so we do something trickier
  // to get a vector in the right order.
  for (Node* node : dependencies_) {
    node->ObjectFiles(files);
  }
}

void Node::FinalOutputs(set<Resource>* outputs) const {
  for (Node* node : dependencies_) {
    node->FinalOutputs(outputs);
  }
}

void Node::LinkFlags(set<string>* flags) const {
  for (Node* node : dependencies_) {
    node->LinkFlags(flags);
  }
}

void Node::CompileFlags(bool cxx, set<string>* flags) const {
  for (Node* node : dependencies_) {
    node->CompileFlags(cxx, flags);
  }
}

string Node::MakefileEscape(const string& str) const {
  return strings::ReplaceAll(str, "$", "$$");
}

void Node::WriteBaseUserTarget(const set<Resource>& deps,
                               Makefile* out) const {
  out->append(target().make_path());
  out->append(":");
  for (const Resource& dep : deps) {
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

void SimpleLibraryNode::DependencyFiles(set<Resource>* files) const {
  Node::DependencyFiles(files);
  for (int i = 0; i < sources_.size(); ++i) {
    files->insert(sources_[i]);
  }
}

void SimpleLibraryNode::ObjectFiles(ObjectFileSet* files) const {
  Node::ObjectFiles(files);
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
