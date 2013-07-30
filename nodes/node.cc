// Copyright 2013
// Author: Christopher Van Arsdale

#include <string>
#include <vector>
#include "common/log/log.h"
#include "common/file/fileutil.h"
#include "common/strings/path.h"
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
}

// Mutators
void Node::AddDependency(const TargetInfo& other) {
  dependencies_.push_back(new TargetInfo(other));
}

// static
void Node::ParseRepeatedString(const BuildFileNode& input,
                               const string& key,
                               vector<string>* out) {
  const Json::Value& array = input.object()[key];
  if (!array.isNull()) {
    CHECK(array.isArray()) << "Expecting array for key " << key << ": "
                           << input.object();
    for (int i = 0; i < array.size(); ++i) {
      const Json::Value& single = array[i];
      CHECK(single.isString()) << "Expecting string for item of " << key << ": "
                               << input.object();
      out->push_back(single.asString());
    }
  }
}

void Node::ParseRepeatedFiles(const BuildFileNode& input,
                              const string& key,
                              vector<string>* out) {
  vector<string> temp;
  ParseRepeatedString(input, key, &temp);
  for (const string& file : temp) {
    int size = out->size();
    CHECK(file::Glob(strings::JoinPath(target().dir(), file), out))
        << "Could not run glob().";
    if (out->size() == size) {
      LOG(FATAL) << "No matched files: " << file
                 << " for target " << target().full_path();
    }
  }
}

// static
bool Node::ParseStringField(const BuildFileNode& input,
                            const string& key,
                            string* field) {
  const Json::Value& json_field = input.object()[key];
  if (!json_field.isString()) {
    return false;
  }
  *field = json_field.asString();
  return true;
}

}  // namespace repobuild
