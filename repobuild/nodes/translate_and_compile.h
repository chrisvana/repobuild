// Copyright 2013
// Author: Christopher Van Arsdale
// Modified by Mark Vandevoorde

#ifndef _REPOBUILD_NODES_TRANSLATE_AND_COMPILE_H__
#define _REPOBUILD_NODES_TRANSLATE_AND_COMPILE_H__

#include <string>
#include <vector>
#include "repobuild/nodes/node.h"

namespace repobuild {
class CCLibraryNode;
class GenShNode;
class GoLibraryNode;
class JavaLibraryNode;
class PyLibraryNode;

/**
 * A TranslateAndCompileNode expands into two or more nodes: a gen_sh
 * to run a source-to-source translator and second node to compile the
 * generated source.  The translator may generate output source for
 * multiple languages, in which case multiple nodes are generated:
 * E.g., both a cc_library node and a java_library node.
 *
 * TranslateAndCompileNode is intended to be used for tools such as
 * Protobuf and Rpcz.
 *
 * An example:

 { "translate_and_compile": {
     "name": "simple_proto",
     "sources": [ "simple.proto" ],
     "translator": "protoc",
     "generate_cc": true,
     "java_classnames": [ "..." ],
     "cc": {
       "header_suffixes": [ ".pb.h" ],   // becomes simple.pb.h
       "source_suffixes": [ ".pb.cc" ],  // becomes simple.pb.cc
       "support_library": "//third_party/protobuf:cc_proto",
       "translator_args": "--cpp_out=$TRANSLATOR_OUTPUT"
     },
     "java": {
       "support_library": "//third_party/protobuf:java_proto",
       "translator_args": "--java_out=$TRANSLATOR_OUTPUT"
     },
     "go": {
       "support_library": "//third_party/protobuf:go_proto",
       "translator_args": "--go_out=$TRANSLATOR_OUTPUT"
     },
     "py": {
       "support_library": "//third_party/protobuf:py_proto",
       "translator_args": "--py_out=$TRANSLATOR_OUTPUT"
     }
 } }
 */
class TranslateAndCompileNode : public Node {
 public:
  TranslateAndCompileNode(const TargetInfo& t,
                   const Input& i,
                   DistSource* s)
      : Node(t, i, s),
        gen_node_(NULL),
        cc_node_(NULL),
        java_node_(NULL),
        go_node_(NULL),
        py_node_(NULL) {
  }
  virtual ~TranslateAndCompileNode() {}
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void PostParse();
  virtual void LocalWriteMake(Makefile* out) const;

 private:
  void FindPrefixes(const std::vector<Resource>& input_files,
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

  std::string translator_;  // the name of the translator
  GenShNode* gen_node_;		// the node to run the translator

  // Nodes to compile translator output.  Null means not needed.
  CCLibraryNode* cc_node_;
  JavaLibraryNode* java_node_;
  GoLibraryNode* go_node_;
  PyLibraryNode* py_node_;
};

}  // namespace repobuild

# endif  // _REPOBUILD_NODES_TRANSLATE_AND_COMPILE_H__
