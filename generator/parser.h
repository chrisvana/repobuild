// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_GENERATOR_PARSER_H__
#define _REPOBUILD_GENERATOR_PARSER_H__

#include <vector>
#include <memory>

namespace repobuild {

class BuildFile;
class Input;
class Node;

class Parser {
 public:
  Parser();
  ~Parser();

  // Mutators.
  void Parse(const Input& input);

  // Accessors.
  const Input& input() const { return *input_; }
  const std::vector<Node*>& nodes() const { return nodes_; }
  const std::vector<BuildFile*> files() const { return builds_; }

 private:
  void Reset();

  std::unique_ptr<Input> input_;
  std::vector<Node*> nodes_;
  std::vector<BuildFile*> builds_;
};

}  // namespace repobuild

#endif // _REPOBUILD_GENERATOR_PARSER_H__
