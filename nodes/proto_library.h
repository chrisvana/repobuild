// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_PROTO_LIBRARY_H__
#define _REPOBUILD_NODES_PROTO_LIBRARY_H__

#include <string>
#include <vector>
#include "repobuild/nodes/node.h"

namespace repobuild {

class ProtoLibraryNode : public Node {
 public:
  ProtoLibraryNode(const TargetInfo& t,
                   const Input& i)
      : Node(t, i) {
  }
  virtual ~ProtoLibraryNode() {}
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void LocalWriteMake(Makefile* out) const {
    WriteBaseUserTarget(out);
  }

 private:
  void FindProtoPrefixes(const std::vector<Resource>& input_files,
                         std::vector<Resource>* prefixes) const;

  Node* GenerateGo(const std::vector<Resource>& input_prefixes,
                   std::vector<std::string>* outputs,
                   BuildFile* file);
  Node* GenerateCpp(const std::vector<Resource>& input_prefixes,
                    std::vector<std::string>* outputs,
                    BuildFile* file);
  Node* GeneratePython(const std::vector<Resource>& input_prefixes,
                       std::vector<std::string>* outputs,
                       BuildFile* file);
  Node* GenerateJava(const std::vector<Resource>& input_prefixes,
                     const std::vector<std::string>& java_classnames,
                     std::vector<std::string>* outputs,
                     BuildFile* file);
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_PROTO_LIBRARY_H__
