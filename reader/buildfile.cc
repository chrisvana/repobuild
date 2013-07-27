// Copyright 2013
// Author: Christopher Van Arsdale

#include <string>
#include <vector>
#include "common/log/log.h"
#include "reader/buildfile.h"
#include "json/include/json/json.h"

namespace repobuild {

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

void BuildFile::Parse(const std::string& input) {
  Json::Value root;
  Json::Reader reader;
  bool ok = reader.parse(input, root);
  if (!ok) {
    LOG(FATAL) << "Reader error: "
               << reader.getFormattedErrorMessages();
  }
  CHECK(root.isArray()) << root;

  for (int i = 0; i < root.size(); ++i) {
    const Json::Value& value = root[i];
    CHECK(value.isObject()) << "Unexpected: " << value;
    nodes_.push_back(new BuildFileNode(value));
  }
}

}  // namespace repobuild
