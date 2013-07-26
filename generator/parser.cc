// Copyright 2013
// Author: Christopher Van Arsdale

#include <vector>
#include <set>
#include "common/log/log.h"
#include "common/file/fileutil.h"
#include "common/strings/path.h"
#include "env/input.h"
#include "generator/parser.h"
#include "json/include/json/json.h"
#include "nodes/node.h"
#include "nodes/allnodes.h"
#include "reader/buildfile.h"

namespace repobuild {

Parser::Parser() {}

Parser::~Parser() {
  Reset();
}

void Parser::Parse(const Input& input) {
  Reset();

  std::set<std::string> build_files;
  std::vector<TargetInfo> targets;
  for (int i = 0; i < input.build_targets().size(); ++i) {
    targets.push_back(
        TargetInfo(input.build_targets()[i],
                   strings::JoinPath(input.root_dir(), "BUILD")));
    if (build_files.insert(targets.back().build_file()).second) {
      builds_.push_back(new BuildFile(targets.back().build_file()));
    }
  }

  for (BuildFile* file : builds_) {
    std::string filestr = file::ReadFileToStringOrDie(file->filename());
    file->Parse(filestr);
    for (BuildFileNode* node : file->nodes()) {
      LOG(INFO) << node->object();
    }
  }

  LOG(FATAL) << "TODO";
}

void Parser::Reset() {
  input_.reset();
  for (auto it : nodes_) {
    delete it;
  }
  nodes_.clear();
  for (auto it : builds_) {
    delete it;
  }
  builds_.clear();
}

}  // namespace repobuild
