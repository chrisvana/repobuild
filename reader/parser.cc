// Copyright 2013
// Author: Christopher Van Arsdale

#include <iostream>
#include <string>
#include <set>
#include <queue>
#include <vector>
#include "common/log/log.h"
#include "common/file/fileutil.h"
#include "common/strings/path.h"
#include "repobuild/env/input.h"
#include "repobuild/json/json.h"
#include "repobuild/nodes/node.h"
#include "repobuild/nodes/allnodes.h"
#include "repobuild/reader/buildfile.h"
#include "repobuild/reader/parser.h"

namespace repobuild {
namespace {
Node* ParseNode(NodeBuilder* builder,
                BuildFile* file,
                BuildFileNode* file_node,
                const Input& input,
                const std::string& key) {
  TargetInfo target;
  const Json::Value& value = file_node->object()[key];
  if (value.isMember("name")) {
    if (!value["name"].isString()) {
      LOG(FATAL) << "Require string \"name\": "
                 << "File: " << file->filename()
                 << ": "<< file_node->object();
    }
    target = TargetInfo(":" + value["name"].asString(), file->filename());
  } else {
    target = TargetInfo(":" + file->NextName(), file->filename());
  }
  Node* node = builder->NewNode(target, input);

  BuildFileNode subnode(value);
  node->Parse(file, subnode);
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
    // Seed initial targets.
    std::queue<std::string> to_process;
    std::set<std::string> queued_targets;
    for (const TargetInfo& info : input.build_targets()) {
      const std::string& cleaned = info.full_path();
      if (queued_targets.insert(cleaned).second) {
        to_process.push(cleaned);
      }
    }

    // Parse our dependency graph using something like BFS.
    while (!to_process.empty()) {
      // Get the next target.
      TargetInfo target;
      {
        std::string current = to_process.front();
        std::cout << "Processing: " << current << std::endl;
        to_process.pop();
        target = TargetInfo(current);
      }

      // Add the build file if we have not yet processed it.
      AddFile(input, target.build_file());

      // Expand the target if we managed to find one in that BUILD file.
      Node* node = nodes_[target.full_path()];
      if (node == NULL) {
        LOG(FATAL) << "Could not find target: " << target.full_path();
      }
      for (const TargetInfo* dep : node->dependencies()) {
        if (queued_targets.insert(dep->full_path()).second) {
          to_process.push(dep->full_path());
        }
      }
    }

    // Record which ones came from our inputs.
    for (auto it : nodes_) {
      Node* node = it.second;
      const TargetInfo& target = node->target();
      if (input.contains_target(target.full_path())) {
        inputs_.push_back(node);
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
  BuildFile* AddFile(const Input& input, const std::string& filename) {
    // Skip processing if we have done it already.
    if (build_files_.find(filename) != build_files_.end()) {
      return build_files_.find(filename)->second;
    }

    // Initialize our parents.
    BuildFile* file =  new BuildFile(filename);
    build_files_[filename] = file;
    ProcessParent(input, file);  // inherit anything we need to from parents.

    // Parse the BUILD into a structured format.
    std::string filestr = file::ReadFileToStringOrDie(file->filename());
    file->Parse(filestr);

    // Parse all of the elements of the build file.
    std::vector<Node*> nodes;
    for (BuildFileNode* node : file->nodes()) {
      CHECK(node->object().isObject()) << "Expected object: " << node->object();
      for (std::string key : node->object().getMemberNames()) {

        // Find the right parser.
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

        // Actually do the parsing.
        SaveNode(ParseNode(builder, file, node, input, key), &nodes);
      }
    }

    // Connect any additional dependencies from the build file.
    // TODO(cvanarsdale): We can only have one at the moment, due to how these
    // get added.
    for (const std::string& additional_dep : file->base_dependencies()) {
      Node* base_dep = nodes_[additional_dep];
      CHECK(base_dep);
      for (Node* node : nodes) {
        if (node->target().full_path() != additional_dep) {
          node->AddDependency(base_dep->target());
        }
      }
    }
    return file;
  }

  void SaveNode(Node* node,
                std::vector<Node*>* all) {
    // Gather all subnodes + this parent node.
    std::vector<Node*> nodes;
    node->ExtractSubnodes(&nodes);
    nodes.push_back(node);

    for (Node* out_node : nodes) {
      const std::string& target = out_node->target().full_path();
      if (nodes_.find(target) != nodes_.end()) {
        LOG(FATAL) << "Duplicate target: " << target;
      }

      // Save the output
      all->push_back(out_node);
      nodes_[target] = out_node;
      targets_[target] = out_node->target();
    }
  }

  void ProcessParent(const Input& input, BuildFile* child) {
    BuildFile* current = child;
    while (true) {
      std::string current_dir = strings::PathDirname(current->filename());
      if (current_dir == "." || current_dir == input.root_dir()) {
        break;
      }

      std::string parent_file = strings::JoinPath(
          strings::JoinPath(current_dir, ".."), "BUILD");
      BuildFile* parent = AddFile(input, parent_file);
      child->MergeParent(parent);
      current = parent;
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
