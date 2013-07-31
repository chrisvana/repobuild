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
                               vector<string>* out) const {
  const Json::Value& array = input.object()[key];
  if (!array.isNull()) {
    CHECK(array.isArray()) << "Expecting array for key " << key << ": "
                           << input.object();
    for (int i = 0; i < array.size(); ++i) {
      const Json::Value& single = array[i];
      CHECK(single.isString()) << "Expecting string for item of " << key << ": "
                               << input.object();
      out->push_back(ParseSingleString(single.asString()));
    }
  }
}

void Node::ParseRepeatedFiles(const BuildFileNode& input,
                              const string& key,
                              vector<string>* out) const {
  vector<string> temp;
  ParseRepeatedString(input, key, &temp);
  for (const string& file : temp) {
    int size = out->size();
    string glob = strings::JoinPath(target().dir(), file);
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


string Node::ParseSingleString(const string& str) const {
  strings::VarMap vars;
  string tmp = RelativeGenDir();
  vars.Set("$GEN_DIR", tmp);
  vars.Set("$(GEN_DIR)", tmp);
  vars.Set("${GEN_DIR}", tmp);
  return vars.Replace(str);
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
