// Copyright 2013
// Author: Christopher Van Arsdale

#include <string>
#include <vector>
#include "common/log/log.h"
#include "common/file/fileutil.h"
#include "common/strings/path.h"
#include "common/strings/strutil.h"
#include "common/strings/varmap.h"
#include "repobuild/env/input.h"
#include "repobuild/nodes/node.h"
#include "repobuild/reader/buildfile.h"
#include "repobuild/json/json.h"

using std::map;
using std::string;
using std::vector;
using std::set;

namespace repobuild {
namespace {
const Json::Value& GetValue(const BuildFileNode& input, const string& key) {
  const Json::Value* current = &input.object();
  for (const string& subkey : strings::SplitString(key, ".")) {
    if (current->isNull()) {
      break;
    }
    current = &(*current)[subkey];
  }
  return *current;
}
}

void Node::Parse(BuildFile* file, const BuildFileNode& input) {
  CHECK(input.object().isObject());
  vector<string> deps;
  ParseRepeatedString(input, "dependencies", &deps);
  for (int i = 0; i < deps.size(); ++i) {
    dependencies_.push_back(new TargetInfo(deps[i], file->filename()));
  }
  ParseBoolField(input, "strict_file_mode", &strict_file_mode_);
  ParseKeyValueStrings(input, "env", &env_variables_);
}

void Node::WriteMake(const std::vector<const Node*>& all_deps,
                     Makefile* out) const {
  WriteVariables(out->mutable_out());
  WriteMakefile(all_deps, out);
}

void Node::AddDependency(const TargetInfo& other) {
  dependencies_.push_back(new TargetInfo(other));
}

void Node::ParseRepeatedString(const BuildFileNode& input,
                               const string& key,
                               bool relative_gendir,
                               vector<string>* out) const {
  const Json::Value& array = GetValue(input, key);
  if (!array.isNull()) {
    CHECK(array.isArray()) << "Expecting array for key " << key << ": "
                           << input.object();
    for (int i = 0; i < array.size(); ++i) {
      const Json::Value& single = array[i];
      CHECK(single.isString()) << "Expecting string for item of " << key << ": "
                               << input.object();
      out->push_back(ParseSingleString(relative_gendir, single.asString()));
    }
  }
}

void Node::ParseRepeatedFiles(const BuildFileNode& input,
                              const string& key,
                              vector<std::string>* out) const {
  vector<string> temp;
  ParseRepeatedString(input, key, false /* absolute gen dir */, &temp);
  for (const string& file : temp) {
    int size = out->size();

    // TODO(cvanarsdale): hacky.
    string glob;
    if (!strings::HasPrefix(file, GenDir())) {
      glob = strings::JoinPath(target().dir(), file);
    } else {
      glob = file;
    }

    CHECK(file::Glob(glob, out))
        << "Could not run glob("
        << glob
        << "), bad permissions?";
    if (out->size() == size) {
      if (strict_file_mode_) {
        LOG(FATAL) << "No matched files: " << file
                   << " for target " << target().full_path();
      } else {
        out->push_back(glob);
      }
    }
  }
}

bool Node::ParseStringField(const BuildFileNode& input,
                            const string& key,
                            string* field) const {
  const Json::Value& json_field = GetValue(input, key);
  if (!json_field.isString()) {
    return false;
  }
  *field = ParseSingleString(json_field.asString());
  return true;
}

bool Node::ParseBoolField(const BuildFileNode& input,
                          const string& key,
                          bool* field) const {
  const Json::Value& json_field = GetValue(input, key);
  if (!json_field.isBool()) {
    return false;
  }
  *field = json_field.asBool();
  return true;
}


void Node::ParseKeyValueStrings(const BuildFileNode& input,
                                const string& key,
                                map<string, string>* out) const {
  const Json::Value& list = input.object()[key];
  if (list.isNull()) {
    return;
  }
  CHECK(list.isObject())
      << "KeyValue list (\"" << key
      << "\") must be object in " << target().full_path();
  for (const string& name : list.getMemberNames()) {
    const Json::Value& val = list[name];
    CHECK(val.isString()) << "Value var (\"" << name
                          << "\") must be string in " << target().full_path();
    (*out)[name] = ParseSingleString(false, val.asString());
  }
}

string Node::ParseSingleString(bool relative_gendir,
                               const string& str) const {
  strings::VarMap vars;
  string tmp = (relative_gendir ? RelativeGenDir() : GenDir());
  vars.Set("$GEN_DIR", tmp);
  vars.Set("$(GEN_DIR)", tmp);
  vars.Set("${GEN_DIR}", tmp);

  tmp = target().dir();
  vars.Set("$SRC_DIR", tmp);
  vars.Set("$(SRC_DIR)", tmp);
  vars.Set("${SRC_DIR}", tmp);

  tmp = strings::Repeat(
      "../", strings::NumPathComponents(target().dir()));
  vars.Set("$ROOT_DIR", tmp);
  vars.Set("$(ROOT_DIR)", tmp);
  vars.Set("${ROOT_DIR}", tmp);
  return vars.Replace(str);
}

void Node::EnvVariables(map<string, string>* env) const {
  for (const auto& it : env_variables_) {
    (*env)[it.first] = it.second;
  }
}

void Node::CollectDependencies(const vector<const Node*>& all_deps,
                               set<string>* out) const {
  for (int i = 0; i < all_deps.size(); ++i) {
    vector<string> files;
    all_deps[i]->DependencyFiles(&files);
    for (const string& it : files) { out->insert(it); }
  }
  vector<string> files;
  DependencyFiles(&files);
  for (const string& it : files) {
    out->insert(it);
  }
}

void Node::CollectObjects(const vector<const Node*>& all_deps,
                          vector<string>* out) const {
  set<string> tmp;
  for (const Node* dep : all_deps) {
    vector<string> obj_files;
    dep->ObjectFiles(&obj_files);
    for (const string& it : obj_files) {
      if (tmp.insert(it).second) {
        out->push_back(it);
      }
    }
  }

  {
    vector<string> obj_files;
    ObjectFiles(&obj_files);
    for (const string& it : obj_files) {
      if (tmp.insert(it).second) {
        out->push_back(it);
      }
    }
  }
}

void Node::CollectLinkFlags(const vector<const Node*>& all_deps,
                            set<string>* out) const {
  for (const Node* dep : all_deps) {
    dep->LinkFlags(out);
  }
  LinkFlags(out);
}

void Node::CollectCompileFlags(bool cxx,
                               const vector<const Node*>& all_deps,
                               set<string>* out) const {
  for (const Node* dep : all_deps) {
    dep->CompileFlags(cxx, out);
  }
  CompileFlags(cxx, out);
}

void Node::CollectEnvVariables(
    const std::vector<const Node*>& all_deps,
    std::map<std::string, std::string>* vars) const {
  for (const Node* dep : all_deps) {
    dep->EnvVariables(vars);
  }
  EnvVariables(vars);
}

string Node::GenDir() const { 
  return strings::JoinPath(input().genfile_dir(), target().dir());
}

string Node::RelativeGenDir() const {
  return strings::JoinPath(
      strings::Repeat("../", strings::NumPathComponents(target().dir())) +
      input().genfile_dir(),
      target().dir());
}

string Node::ObjectDir() const { 
  return strings::JoinPath(input().object_dir(), target().dir());
}

string Node::RelativeObjectDir() const {
  return strings::JoinPath(RelativeRootDir() + input().object_dir(),
                           target().dir());
}

string Node::SourceDir() const { 
  return strings::JoinPath(input().source_dir(), target().dir());
}

string Node::RelativeSourceDir() const {
  return strings::JoinPath(RelativeRootDir() + input().source_dir(),
                           target().dir());
}


string Node::RelativeRootDir() const {
  return strings::Repeat("../", strings::NumPathComponents(target().dir()));
}

string Node::MakefileEscape(const string& str) const {
  return strings::ReplaceAll(str, "$", "$$");
}

void Node::WriteBaseUserTarget(const set<string>& deps,
                               Makefile* out) const {
  out->append(target().make_path());
  out->append(":");
  for (const string& dep : deps) {
    out->append(" ");
    out->append(dep);
  }
  for (const TargetInfo* dep : dependencies()) {
    out->append(" ");
    out->append(dep->make_path());
  }
  out->append("\n\n.PHONY: ");
  out->append(target().make_path());
  out->append("\n\n");
}

void Node::MakeVariable::WriteMake(std::string* out) const {
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

void SimpleLibraryNode::DependencyFiles(vector<string>* files) const {
  Node::DependencyFiles(files);
  for (int i = 0; i < sources_.size(); ++i) {
    files->push_back(sources_[i]);
  }
}

void Makefile::StartRule(const std::string& rule,
                         const std::string& dependencies) {
  out_.append("\n");
  out_.append(rule);
  out_.append(": ");
  out_.append(dependencies);
  out_.append("\n");
}

void Makefile::FinishRule() {
  out_.append("\n");
}

void Makefile::WriteCommand(const std::string& command) {
  out_.append("\t");
  if (silent_) {
    out_.append("@");
  }
  out_.append(command);
  out_.append("\n");
}

void Makefile::WriteCommandBestEffort(const std::string& command) {
  out_.append("\t-");  // - == ignore failures
  if (silent_) {
    out_.append("@");
  }
  out_.append(command);
  out_.append("\n");
}

}  // namespace repobuild
