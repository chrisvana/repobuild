// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_READER_PARSER_H__
#define _REPOBUILD_READER_PARSER_H__

#include <vector>
#include <memory>

namespace repobuild {

class BuildFile;
class DistSource;
class Input;
class Node;
class NodeBuilderSet;

class Parser {
 public:
  Parser(const NodeBuilderSet* builder_set /* keeps reference */,
         DistSource* source /* keeps reference */);
  ~Parser();

  // Mutators.
  void Parse(const Input& input);

  // Accessors.
  const Input& input() const { return *input_; }
  const std::vector<Node*>& input_nodes() const { return input_nodes_; }
  const std::vector<Node*>& all_nodes() const { return all_node_vec_; }

  const Node* GetNode(const std::string& target) const {
    auto it = all_nodes_.find(target);
    if (it == all_nodes_.end()) {
      return NULL;
    }
    return it->second;
  }

  const BuildFile* GetBuild(const std::string& file) const {
    auto it = builds_.find(file);
    if (it == builds_.end()) {
      return NULL;
    }
    return it->second;
  }

 private:
  void Reset();

  const NodeBuilderSet* builder_set_;
  DistSource* dist_source_;
  std::unique_ptr<Input> input_;
  std::vector<Node*> input_nodes_, all_node_vec_;
  std::map<std::string, BuildFile*> builds_;
  std::map<std::string, Node*> all_nodes_;
};

}  // namespace repobuild

#endif // _REPOBUILD_READER_PARSER_H__
