// Copyright 2013
// Author: Christopher Van Arsdale

#include <map>
#include <set>
#include <string>
#include <vector>
#include "common/log/log.h"
#include "common/file/fileutil.h"
#include "common/strings/path.h"
#include "common/strings/strutil.h"
#include "common/strings/varmap.h"
#include "repobuild/env/resource.h"
#include "repobuild/reader/buildfile.h"
#include "repobuild/json/json.h"

using std::set;
using std::string;
using std::vector;
using std::map;

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
}  // anonymous namespace

BuildFileNode::BuildFileNode(const Json::Value& object)
    : object_(new Json::Value(object)) {
}

BuildFileNode::~BuildFileNode() {
}

BuildFile::~BuildFile() {
  for (auto it : nodes_) {
    delete it;
  }
}

void BuildFile::Parse(const string& input) {
  Json::Value root;
  Json::Reader reader;
  bool ok = reader.parse(input, root);
  if (!ok) {
    LOG(FATAL) << "Reader error for "
               << filename()
               << ": "
               << reader.getFormattedErrorMessages();
  }
  CHECK(root.isArray()) << root;

  for (int i = 0; i < root.size(); ++i) {
    const Json::Value& value = root[i];
    CHECK(value.isObject()) << "Unexpected: " << value;
    nodes_.push_back(new BuildFileNode(value));
  }
}

string BuildFile::NextName() {
  return strings::StringPrintf("__auto_name_%d", name_counter_++);
}

void BuildFile::MergeParent(BuildFile* parent) {
  for (const string& dep : parent->base_dependencies()) {
    base_deps_.insert(dep);
  }
}

BuildFileNodeReader::BuildFileNodeReader(const BuildFileNode& node)
    : input_(node),
      var_map_true_(new strings::VarMap),
      var_map_false_(new strings::VarMap),
      strict_file_mode_(true) {
}

BuildFileNodeReader::~BuildFileNodeReader() {
}

void BuildFileNodeReader::SetReplaceVariable(bool mode,
                                             const string& original,
                                             const string& replace) {
  strings::VarMap* var = (mode ? var_map_true_.get() : var_map_false_.get());
  var->Set("$" + original, replace);
  var->Set("$(" + original + ")", replace);
  var->Set("${" + original + "}", replace);
}

void BuildFileNodeReader::ParseRepeatedString(const string& key,
                                              bool mode,
                                              vector<string>* output) const {
  const Json::Value& array = GetValue(input_, key);
  if (!array.isNull()) {
    CHECK(array.isArray()) << "Expecting array for key " << key << ": "
                           << input_.object();
    for (int i = 0; i < array.size(); ++i) {
      const Json::Value& single = array[i];
      CHECK(single.isString()) << "Expecting string for item of " << key << ": "
                               << input_.object()
                               << ". Target: " << error_path_;
      output->push_back(RewriteSingleString(mode, single.asString()));
    }
  }
}

void BuildFileNodeReader::ParseKeyValueStrings(
    const string& key,
    map<string, string>* output) const {
  const Json::Value& list = input_.object()[key];
  if (list.isNull()) {
    return;
  }
  CHECK(list.isObject())
      << "KeyValue list (\"" << key
      << "\") must be object in " << error_path_;
  for (const string& name : list.getMemberNames()) {
    const Json::Value& val = list[name];
    CHECK(val.isString()) << "Value var (\"" << name
                          << "\") must be string in " << error_path_;
    (*output)[name] = RewriteSingleString(false, val.asString());
  }
}

bool BuildFileNodeReader::ParseStringField(const string& key,
                                           string* field) const {
  const Json::Value& json_field = GetValue(input_, key);
  if (!json_field.isString()) {
    return false;
  }
  *field = RewriteSingleString(true, json_field.asString());
  return true;
}

// Parse files.
void BuildFileNodeReader::ParseRepeatedFiles(const string& key,
                                             vector<Resource>* output) const {
  vector<string> temp;
  ParseRepeatedString(key, &temp);
  for (const string& file : temp) {
    // TODO(cvanarsdale): hacky. Probably better to make build file have more
    // complex syntax. E.g.:
    // build_file_list = [ "local.cc", { "gen": "generated.cc" }, ... ]
    string glob = strings::JoinPath(file_path_, file);
    for (const string& prefix : abs_prefix_) {
      if (strings::HasPrefix(file, prefix)) {
        glob = file;
        break;
      }
    }

    vector<string> tmp;
    CHECK(file::Glob(glob, &tmp))
        << "Could not run glob(" << glob << "), bad filesystem permissions?";
    if (tmp.empty()) {
      if (strict_file_mode_) {
        LOG(FATAL) << "No matched files: " << file
                   << " for target " << error_path_;
      } else {
        output->push_back(Resource::FromRootPath(glob));
      }
    } else {
      for (const string& it : tmp) {
        output->push_back(Resource::FromRootPath(it));
      }
    }
  }
}

bool BuildFileNodeReader::ParseBoolField(const string& key,
                                         bool* field) const {
  const Json::Value& json_field = GetValue(input_, key);
  if (!json_field.isBool()) {
    return false;
  }
  *field = json_field.asBool();
  return true;
}

string BuildFileNodeReader::RewriteSingleString(bool mode,
                                                const string& str) const {
  return (mode ? var_map_true_.get() : var_map_false_.get())->Replace(str);
}

}  // namespace repobuild
