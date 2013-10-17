// Copyright 2013
// Author: Christopher Van Arsdale

#include <stdio.h>
#include <string>
#include <iostream>
#include "common/log/log.h"
#include "common/util/shell.h"
#include "repobuild/nodes/plugin.h"
#include "repobuild/third_party/json/json.h"

using std::string;

namespace repobuild {

void PluginNode::Parse(BuildFile* file, const BuildFileNode& input) {
  Node::Parse(file, input);
  VLOG(1) << "Registering plugin: " << target().local_path();
  file->RegisterKey("plugin:" + target().local_path(),
                    target().full_path());
  current_reader()->ParseStringField("command", false /* no cd */, &command_);
}

void PluginNode::LocalWriteMake(Makefile* out) const {
  WriteBaseUserTarget(out);
}

bool PluginNode::ExpandBuildFileNode(BuildFile* file, BuildFileNode* node) {
  if (command_.empty()) {
    return false;
  }

  // Execute subprocess.
  string stdout;
  Json::FastWriter writer;
  int status = util::Execute(writer.write(node->object()),
                             command_.c_str(),
                             &stdout);
  if (status != 0) {
    LOG(FATAL) << "Plugin: " << target().full_path()
               << " returned non-zero (" << status << ") for command "
               << command_;
  }

  // Parse our stdout.
  Json::Value root;
  Json::Reader reader;
  VLOG(1) << "Plugin json: " << stdout;
  bool ok = reader.parse(stdout, root);
  if (!ok) {
    LOG(FATAL) << "Plugin generated invalid json.\n\nIn "
               << target().full_path()
               << ":\n "
               << reader.getFormattedErrorMessages()
               << "\n\n(check for missing/spurious commas).\n\n"
               << "Json was:\n"
               << stdout;
  }
  CHECK(root.isObject()) << root;
  if (root != node->object()) {
    node->Reset(root);
    return true;
  }
  return false;
}

}  // namespace repobuild
