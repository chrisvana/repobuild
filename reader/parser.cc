// Copyright 2013
// Author: Christopher Van Arsdale

#include <string>
#include <set>
#include <queue>
#include <vector>
#include "common/log/log.h"
#include "common/file/fileutil.h"
#include "common/strings/path.h"
#include "env/input.h"
#include "json/json.h"
#include "nodes/node.h"
#include "nodes/allnodes.h"
#include "reader/buildfile.h"
#include "reader/parser.h"

namespace repobuild {
namespace {
Node* ParseNode(NodeBuilder* builder,
                BuildFile* file,
                BuildFileNode* file_node,
                const std::string& key) {
  const Json::Value& value = file_node->object()[key];
  CHECK(value.isMember("name") && value["name"].isString())
      << "Require string \"name\": "
      << "File: " << file->filename()
      << ": "<< file_node->object();
  TargetInfo target(":" + value["name"].asString(), file->filename());
  Node* node = builder->NewNode(target);

  BuildFileNode subnode(value);
  node->Parse(*file, subnode);
  return node;
}

class Graph {
 public:
  Graph() {
    NodeBuilder::GetAll(&node_builders_);
  }

  ~Graph() {
    for (auto it : node_builders_) {
      delete it;
    }
    for (auto it : build_files_) {
      delete it.second;
    }
    for (auto it : nodes_) {
      delete it.second;
    }
  }

  void Parse(const Input& input) {
    // Initial targets.
    std::queue<std::string> to_process;
    std::set<std::string> queued_targets;
    std::set<std::string> input_targets;
    for (std::string target : input.build_targets()) {
      TargetInfo info(target, "BUILD");
      const std::string& cleaned = info.full_path();
      input_targets.insert(cleaned);
      if (queued_targets.insert(cleaned).second) {
        to_process.push(cleaned);
      }
    }

    while (!to_process.empty()) {
      // Get the next target.
      TargetInfo target;
      {
        std::string current = to_process.front();
        LOG(INFO) << "Processing: " << current;
        to_process.pop();
        target = TargetInfo(current);
      }

      // Add the build file if we have not yet processed it.
      if (build_files_.find(target.build_file()) == build_files_.end()) {
        AddFile(target.build_file());
      }

      // Expand the target if we managed to find one in that BUILD file.
      Node* node = nodes_[target.full_path()];
      if (node == NULL) {
        LOG(FATAL) << "Could not find target: " << target.full_path();
      }
      if (input_targets.find(target.full_path()) != input_targets.end()) {
        inputs_.push_back(node);
      }
      for (const TargetInfo* dep : node->dependencies()) {
        if (queued_targets.insert(dep->full_path()).second) {
          to_process.push(dep->full_path());
        }
      }
    }
  }

  void ExtractBuildFiles(std::map<std::string, BuildFile*>* out) {
    *out = build_files_;
    build_files_.clear();
  }

  void ExtractNodes(std::vector<Node*>* inputs,
                    std::map<std::string, Node*>* nodes) {
    *inputs = inputs_;
    *nodes = nodes_;
    nodes_.clear();
    inputs_.clear();
  }

 private:
  void AddFile(const std::string& filename) {
    BuildFile* file =  new BuildFile(filename);
    build_files_[filename] = file;
    std::string filestr = file::ReadFileToStringOrDie(file->filename());
    file->Parse(filestr);

    for (BuildFileNode* node : file->nodes()) {
      CHECK(node->object().isObject()) << "Expected object: " << node->object();
      for (std::string key : node->object().getMemberNames()) {
        NodeBuilder* builder = NULL;
        for (NodeBuilder* b : node_builders_) {
          if (b->Name() == key) {
            builder = b;
            break;
          }
        }
        if (builder == NULL) {
          LOG(FATAL) << "Uknown build rule: " << key;
        }

        Node* out_node = ParseNode(builder, file, node, key);
        const std::string& target = out_node->target().full_path();
        if (nodes_.find(target) != nodes_.end()) {
          LOG(FATAL) << "Duplicate target: " << target;
        }

        nodes_[target] = out_node;
        targets_[target] = out_node->target();
      }
    }
  }

  std::vector<NodeBuilder*> node_builders_;

  std::map<std::string, BuildFile*> build_files_;
  std::map<std::string, TargetInfo> targets_;
  std::map<std::string, Node*> nodes_;
  std::vector<Node*> inputs_;
};
}

Parser::Parser() {}

Parser::~Parser() {
  Reset();
}

void Parser::Parse(const Input& input) {
  Reset();

  Graph graph;
  graph.Parse(input);
  graph.ExtractBuildFiles(&builds_);
  graph.ExtractNodes(&input_nodes_, &all_nodes_);
  for (auto it : all_nodes_) {
    all_node_vec_.push_back(it.second);
  }
}

void Parser::Reset() {
  input_.reset();
  for (auto it : all_nodes_) {
    delete it.second;
  }
  input_nodes_.clear();
  all_nodes_.clear();
  all_node_vec_.clear();
  for (auto it : builds_) {
    delete it.second;
  }
  builds_.clear();
}

}  // namespace repobuild
