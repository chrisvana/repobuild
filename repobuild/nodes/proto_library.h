// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_NODES_PROTO_LIBRARY_H__
#define _REPOBUILD_NODES_PROTO_LIBRARY_H__

#include <string>
#include <vector>
#include "repobuild/nodes/node.h"

namespace repobuild {
class CCLibraryNode;
class GenShNode;
class GoLibraryNode;
class JavaLibraryNode;
class PyLibraryNode;

class ProtoLibraryNode : public Node {
 public:
  ProtoLibraryNode(const TargetInfo& t,
                   const Input& i,
                   DistSource* s)
      : Node(t, i, s),
        gen_node_(NULL),
        cc_node_(NULL),
        java_node_(NULL),
        go_node_(NULL),
        py_node_(NULL) {
  }
  virtual ~ProtoLibraryNode() {}
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void PostParse();
  virtual void LocalWriteMake(Makefile* out) const;

 private:
  void FindProtoPrefixes(const std::vector<Resource>& input_files,
                         std::vector<Resource>* prefixes) const;
  void AdditionalDependencies(BuildFile* file,
                              const std::string& dep_field,
                              const std::string& default_dep_value,
                              Node* node);
  void GenerateGo(const std::vector<Resource>& input_prefixes,
                  std::vector<Resource>* outputs,
                  BuildFile* file);
  void GenerateCpp(const std::vector<Resource>& input_prefixes,
                   std::vector<Resource>* outputs,
                   BuildFile* file);
  void GeneratePython(const std::vector<Resource>& input_prefixes,
                      std::vector<Resource>* outputs,
                      BuildFile* file);
  void GenerateJava(BuildFile* file,
                    const BuildFileNode& input,
                    const std::vector<Resource>& input_prefixes,
                    const std::vector<std::string>& java_classnames,
                    std::vector<Resource>* outputs);
  virtual bool IncludeChildDependency(DependencyCollectionType type,
                                      LanguageType lang,
                                      Node* node) const;

  GenShNode* gen_node_;
  CCLibraryNode* cc_node_;
  JavaLibraryNode* java_node_;
  GoLibraryNode* go_node_;
  PyLibraryNode* py_node_;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_PROTO_LIBRARY_H__
