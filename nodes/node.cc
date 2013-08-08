// Copyright 2013
// Author: Christopher Van Arsdale

#include <string>
#include <vector>
#include "common/log/log.h"
#include "common/file/fileutil.h"
#include "common/strings/path.h"
#include "common/strings/strutil.h"
#include "common/strings/varmap.h"
#include "env/input.h"
#include "nodes/node.h"
#include "repobuild/reader/buildfile.h"
#include "repobuild/json/json.h"

using std::string;
using std::vector;
using std::set;

namespace repobuild {

void Node::Parse(BuildFile* file, const BuildFileNode& input) {
  CHECK(input.object().isObject());
  vector<string> deps;
  ParseRepeatedString(input, "dependencies", &deps);
  for (int i = 0; i < deps.size(); ++i) {
    dependencies_.push_back(new TargetInfo(deps[i], file->filename()));
  }
  ParseBoolField(input, "strict_file_mode", &strict_file_mode_);
}

// Mutators
void Node::AddDependency(const TargetInfo& other) {
  dependencies_.push_back(new TargetInfo(other));
}

void Node::ParseRepeatedString(const BuildFileNode& input,
                               const string& key,
                               bool relative_gendir,
                               vector<string>* out) const {
  const Json::Value& array = input.object()[key];
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
  const Json::Value& json_field = input.object()[key];
  if (!json_field.isString()) {
    return false;
  }
  *field = ParseSingleString(json_field.asString());
  return true;
}

bool Node::ParseBoolField(const BuildFileNode& input,
                          const string& key,
                          bool* field) const {
  const Json::Value& json_field = input.object()[key];
  if (!json_field.isBool()) {
    return false;
  }
  *field = json_field.asBool();
  return true;
}

string Node::ParseSingleString(bool relative_gendir,
                               const string& str) const {
  strings::VarMap vars;
  string tmp = (relative_gendir ? RelativeGenDir() : GenDir());
  vars.Set("$GEN_DIR", tmp);
  vars.Set("$(GEN_DIR)", tmp);
  vars.Set("${GEN_DIR}", tmp);
  return vars.Replace(str);
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
                          set<string>* out) const {
  for (const Node* dep : all_deps) {
    vector<string> obj_files;
    dep->ObjectFiles(&obj_files);
    for (const string& it : obj_files) { out->insert(it); }
  }
  {
    vector<string> obj_files;
    ObjectFiles(&obj_files);
    for (const string& it : obj_files) { out->insert(it); }
  }
}

void Node::CollectLinkFlags(const vector<const Node*>& all_deps,
                            set<string>* out) const {
  for (const Node* dep : all_deps) {
    dep->LinkFlags(out);
  }
  LinkFlags(out);
}

string Node::GenDir() const { 
  return strings::JoinPath(input().genfile_dir(), target().dir());
}

string Node::RelativeGenDir() const {
  int components = strings::NumPathComponents(target().dir());
  string output;
  for (int i = 0; i < components; ++i) {
    output += "../";
  }
  output += input().genfile_dir();
  return strings::JoinPath(output, target().dir());
}

std::string Node::MakefileEscape(const std::string& str) const {
  return strings::ReplaceAll(str, "$", "$$");
}

}  // namespace repobuild
