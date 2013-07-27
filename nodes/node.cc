// Copyright 2013
// Author: Christopher Van Arsdale

#include "common/log/log.h"
#include "nodes/node.h"
#include "reader/buildfile.h"
#include "json/json.h"

namespace repobuild {

void Node::Parse(const BuildFile& file, const BuildFileNode& input) {
  CHECK(input.object().isObject());
  std::vector<std::string> deps;
  ParseRepeatedString(input, "dependencies", &deps);
  for (int i = 0; i < deps.size(); ++i) {
    dependencies_.push_back(new TargetInfo(deps[i], file.filename()));
  }
}

// static
void Node::ParseRepeatedString(const BuildFileNode& input,
                               const std::string& key,
                               std::vector<std::string>* out) {
 
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

}  // namespace repobuild
