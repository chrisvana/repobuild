// Copyright 2013
// Author: Christopher Van Arsdale

#include <string>
#include <set>
#include <iterator>
#include <vector>
#include "common/log/log.h"
#include "common/file/fileutil.h"
#include "common/strings/path.h"
#include "common/strings/strutil.h"
#include "repobuild/env/input.h"
#include "repobuild/nodes/cc_library.h"
#include "repobuild/nodes/gen_sh.h"
#include "repobuild/nodes/go_library.h"
#include "repobuild/nodes/java_library.h"
#include "repobuild/nodes/proto_library.h"
#include "repobuild/nodes/py_library.h"
#include "repobuild/reader/buildfile.h"

using std::vector;
using std::string;
using std::set;

namespace repobuild {

void ProtoLibraryNode::Parse(BuildFile* file, const BuildFileNode& input) {
  Node::Parse(file, input);

  // Read the input files.
  vector<Resource> input_files;
  current_reader()->ParseRepeatedFiles("sources", &input_files);
  if (input_files.empty()) {
    LOG(FATAL) << "proto_library requires input .proto files: "
               << target().full_path();
  }
  vector<Resource> input_prefixes;
  FindProtoPrefixes(input_files, &input_prefixes);

  // Generate the proto files using protoc.
  GenShNode* gen = new GenShNode(
      target().GetParallelTarget(file->NextName(target().local_path())),
      Node::input());
  for (const TargetInfo& dep : dep_targets()) {
    gen->AddDependencyTarget(dep);
  }
  gen->SetCd(false);
  AddSubNode(gen);

  string build_cmd = "protoc";

  // Find all of the output files.
  vector<string> outputs;

  bool generate_cc = false;
  bool generate_java = false;
  bool generate_python = false;
  bool generate_go = false;

  current_reader()->ParseBoolField("generate_cc", &generate_cc);
  current_reader()->ParseBoolField("generate_java", &generate_java);
  current_reader()->ParseBoolField("generate_python", &generate_python);
  current_reader()->ParseBoolField("generate_go", &generate_go);

  // c++
  if (generate_cc) {
    build_cmd += " --cpp_out=" + Node::input().genfile_dir();
    Node* cpp = GenerateCpp(input_prefixes, &outputs, file);
    cpp->AddDependencyTarget(gen->target());

    // Figure out our cc base library dependencies.
    string dep;
    if (!current_reader()->ParseStringField("proto_cc_dep", &dep)) {
      dep = Node::input().default_cc_proto();
    }
    cpp->AddDependencyTarget(TargetInfo(dep, file->filename()));
  }

  // java
  if (generate_java) {
    build_cmd += " --java_out=" + Node::input().genfile_dir();
    vector<string> java_classnames;
    current_reader()->ParseRepeatedString("java_classnames", &java_classnames);
    Node* java = GenerateJava(file, input, input_prefixes,
                              java_classnames, &outputs);
    java->AddDependencyTarget(gen->target());

    // Figure out our cc base library dependencies.
    string dep;
    if (!current_reader()->ParseStringField("proto_java_dep", &dep)) {
      dep = Node::input().default_java_proto();
    }
    java->AddDependencyTarget(TargetInfo(dep, file->filename()));
  }

  // python
  if (generate_python) {
    build_cmd += " --python_out=" + Node::input().genfile_dir();
    Node* python = GeneratePython(input_prefixes, &outputs, file);
    python->AddDependencyTarget(gen->target());

    // Figure out our cc base library dependencies.
    string dep;
    if (!current_reader()->ParseStringField("proto_py_dep", &dep)) {
      dep = Node::input().default_py_proto();
    }
    python->AddDependencyTarget(TargetInfo(dep, file->filename()));
  }

  // go
  if (generate_go) {
    build_cmd += " --go_out=" + Node::input().genfile_dir();
    Node* go = GenerateGo(input_prefixes, &outputs, file);
    go->AddDependencyTarget(gen->target());

    // Figure out our cc base library dependencies.
    string dep;
    if (!current_reader()->ParseStringField("proto_go_dep", &dep)) {
      dep = Node::input().default_go_proto();
    }
    go->AddDependencyTarget(TargetInfo(dep, file->filename()));
  }

  build_cmd += " " + strings::JoinWith(
      " ",
      "-I" + Node::input().root_dir(),
      "-I" + Node::input().genfile_dir(),
      "-I" + Node::input().source_dir(),
      "-I" + strings::JoinPath(Node::input().source_dir(),
                               Node::input().genfile_dir()));  
  build_cmd += " " + strings::JoinAll(input_files, " ");

  gen->Set(build_cmd, "", input_files, outputs);
}

void ProtoLibraryNode::FindProtoPrefixes(const vector<Resource>& input_files,
                                         vector<Resource>* prefixes) const {
  // Find all of the proto basenames.
  for (const Resource& input_file : input_files) {
    const string& file = input_file.path();

    // Make sure the file is within our current directory.
    if (!strings::HasPrefix(file, target().dir())) {
      LOG(FATAL) << "proto_library requires proto "
                 << "files exist in this directory tree: "
                 << file << " vs " << target().dir();
    }

    // Check the file suffix.
    if (!strings::HasSuffix(file, ".proto") &&
        !strings::HasSuffix(file, ".protodevel")) {
      LOG(FATAL) << "Expected .proto suffix: "
                 << file
                 << " (from target " << target().full_path() << ").";
    }

    prefixes->push_back(Resource::FromLocalPath(
        input_file.dirname(),
        input_file.basename().substr(0, input_file.basename().rfind('.'))));
  }
}

Node* ProtoLibraryNode::GenerateCpp(const vector<Resource>& input_prefixes,
                                    vector<string>* outputs,
                                    BuildFile* file) {
  vector<Resource> cc_sources, cc_headers;

  for (const Resource& prefix : input_prefixes) {
    string cpp_file = prefix.path() + ".pb.cc";
    string hpp_file = prefix.path() + ".pb.h";

    // Relative to BUILD file:
    outputs->push_back(cpp_file.substr(target().dir().size() + 1));
    outputs->push_back(hpp_file.substr(target().dir().size() + 1));

    // Relative to root:
    cc_sources.push_back(Resource::FromLocalPath(Node::input().genfile_dir(),
                                                 cpp_file));
    cc_headers.push_back(Resource::FromLocalPath(Node::input().genfile_dir(),
                                                 hpp_file));
  }

  CCLibraryNode* cc_lib = new CCLibraryNode(
      target().GetParallelTarget(file->NextName(target().local_path())),
      Node::input());
  AddSubNode(cc_lib);

  // dummies:
  vector<Resource> objects;
  vector<string> cc_compile_args, header_compile_args;

  cc_lib->Set(cc_sources, cc_headers, objects,
              cc_compile_args, header_compile_args);
  return cc_lib;
}

Node* ProtoLibraryNode::GenerateJava(BuildFile* file,
                                     const BuildFileNode& input,
                                     const vector<Resource>& input_prefixes,
                                     const vector<string>& java_classnames,
                                     vector<string>* outputs) {
  vector<Resource> java_sources;
  if (java_classnames.size() > 0 &&
      java_classnames.size() != input_prefixes.size()) {
    LOG(FATAL) << "java_classnames must match up 1:1 with proto source files: "
               << target().full_path();
  }

  for (int i = 0; i < input_prefixes.size(); ++i) {
    const Resource& prefix = input_prefixes[i];
    string java_classname = (java_classnames.empty() ?
                             strings::Capitalize(prefix.basename()) :
                             java_classnames[i]);
    string java_basename = java_classname + ".java";

    // Relative to BUILD file:
    if (target().dir().size() < prefix.dirname().size()) {
      outputs->push_back(strings::JoinPath(
          prefix.dirname().substr(target().dir().size() + 1),
          java_basename));
    } else {
      outputs->push_back(java_basename);
    }

    // Relative to root:
    java_sources.push_back(
        Resource::FromLocalPath(
            Node::input().genfile_dir(),
            strings::JoinPath(prefix.dirname(), java_basename)));
  }

  JavaLibraryNode* java_lib = new JavaLibraryNode(
      target().GetParallelTarget(file->NextName(target().local_path())),
      Node::input());
  AddSubNode(java_lib);
  java_lib->Set(file, input, java_sources);
  return java_lib;
}

Node* ProtoLibraryNode::GeneratePython(const vector<Resource>& input_prefixes,
                                       vector<string>* outputs,
                                       BuildFile* file) {
  vector<Resource> python_sources;

  for (const Resource& prefix : input_prefixes) {
    // TODO(cvanarsdale): This will sadly break some day.
    string python_file = prefix.path() + "_pb2.py";

    // Relative to BUILD file:
    outputs->push_back(python_file.substr(target().dir().size() + 1));

    // Relative to root:
    python_sources.push_back(Resource::FromLocalPath(
        Node::input().genfile_dir(),
        python_file));
  }

  PyLibraryNode* py_lib = new PyLibraryNode(
      target().GetParallelTarget(file->NextName(target().local_path())),
      Node::input());
  AddSubNode(py_lib);

  py_lib->Set(python_sources);
  return py_lib;
}

Node* ProtoLibraryNode::GenerateGo(const vector<Resource>& input_prefixes,
                                   vector<string>* outputs,
                                   BuildFile* file) {
  vector<Resource> go_sources;

  for (const Resource& prefix : input_prefixes) {
    string go_file = prefix.path() + ".pb.go";

    // Relative to BUILD file:
    outputs->push_back(go_file.substr(target().dir().size() + 1));

    // Relative to root:
    go_sources.push_back(Resource::FromLocalPath(
        Node::input().genfile_dir(),
        go_file));
  }

  GoLibraryNode* go_lib = new GoLibraryNode(
      target().GetParallelTarget(file->NextName(target().local_path())),
      Node::input());
  AddSubNode(go_lib);

  go_lib->Set(go_sources);
  return go_lib;
}

void ProtoLibraryNode::AddDefaultDependency(BuildFile* file,
                                            const BuildFileNode& input,
                                            const string& dep_name,
                                            Node* node) {
  // Figure out our cc base library dependencies.
}

}  // namespace repobuild
