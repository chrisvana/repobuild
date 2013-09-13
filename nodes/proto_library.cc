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
namespace {
const char kProtocVar[] = "PROTOC";
}

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

  // We will generate the proto files using protoc, using a gen_sh node. E.g:
  // protoc --cpp_out=.gen-files --java_out=.gen-files --go_out=.gen-files -I.
  //        -I.gen-files -I.gen-src -I.gen-src/.gen-files testdata/a/a.proto
  gen_node_ = new GenShNode(
      target().GetParallelTarget(file->NextName(target().local_path())),
      Node::input());
  for (const TargetInfo& dep : dep_targets()) {
    gen_node_->AddDependencyTarget(dep);
  }
  gen_node_->SetCd(false);
  AddSubNode(gen_node_);

  // Figure out where protoc lives.
  string protoc_binary = "$" + string(kProtocVar);

  // Figure out which languages to generates.
  bool generate_cc = false;
  bool generate_java = false;
  bool generate_python = false;
  bool generate_go = false;
  current_reader()->ParseBoolField("generate_cc", &generate_cc);
  current_reader()->ParseBoolField("generate_java", &generate_java);
  current_reader()->ParseBoolField("generate_python", &generate_python);
  current_reader()->ParseBoolField("generate_go", &generate_go);

  // Find all of the output files, and build up our protoc command line.
  string build_cmd = protoc_binary;
  vector<string> outputs;

  // c++
  if (generate_cc) {
    build_cmd += " --cpp_out=" + Node::input().genfile_dir();
    GenerateCpp(input_prefixes, &outputs, file);
  }

  // java
  if (generate_java) {
    build_cmd += " --java_out=" + Node::input().genfile_dir();
    vector<string> java_classnames;
    current_reader()->ParseRepeatedString("java_classnames", &java_classnames);
    GenerateJava(file, input, input_prefixes, java_classnames, &outputs);
  }

  // python
  if (generate_python) {
    build_cmd += " --python_out=" + Node::input().genfile_dir();
    GeneratePython(input_prefixes, &outputs, file);
  }

  // go
  if (generate_go) {
    build_cmd += " --go_out=" + Node::input().genfile_dir();
    GenerateGo(input_prefixes, &outputs, file);
  }

  build_cmd += " " + strings::JoinWith(
      " ",
      "-I" + Node::input().root_dir(),
      "-I" + Node::input().genfile_dir(),
      "-I" + Node::input().source_dir(),
      "-I" + strings::JoinPath(Node::input().source_dir(),
                               Node::input().genfile_dir()));  
  build_cmd += " " + strings::JoinAll(input_files, " ");

  gen_node_->Set(build_cmd, "", input_files, outputs);
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

void ProtoLibraryNode::AdditionalDependencies(
    BuildFile* file,
    const string& dep_field,
    const string& default_dep_value,
    Node* node) {
  string dep;
  if (!current_reader()->ParseStringField(dep_field, &dep)) {
    dep = default_dep_value;
  }

  // NB: This is sadly trickier than it should be. The script node that runs
  // protoc depends on everything (so it can make sure protoc is properly
  // built for each langauge). However, we don't want everyone who depends
  // on proto_library to have to depend on every langauge (e.g. cc_library
  // should only depend on cc_node_, etc). That is all covered by
  // IncludeChildDependency below.
  TargetInfo target(dep, file->filename());
  node->AddDependencyTarget(gen_node_->target());
  gen_node_->AddDependencyTarget(target);
  node->AddDependencyTarget(target);
}

void ProtoLibraryNode::GenerateCpp(const vector<Resource>& input_prefixes,
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
  cc_node_ = cc_lib;

  // Dependency fixing
  AdditionalDependencies(file,
                         "proto_cc_dep",
                         input().default_cc_proto(),
                         cc_node_);
}

void ProtoLibraryNode::GenerateJava(BuildFile* file,
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
  java_node_ = java_lib;

  // Dependency fixing
  AdditionalDependencies(file,
                         "proto_java_dep",
                         Node::input().default_java_proto(),
                         java_node_);
}

void ProtoLibraryNode::GeneratePython(const vector<Resource>& input_prefixes,
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
  py_node_ = py_lib;

  // Dependency fixing
  AdditionalDependencies(file,
                         "proto_py_dep",
                         input().default_py_proto(),
                         py_node_);
}

void ProtoLibraryNode::GenerateGo(const vector<Resource>& input_prefixes,
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
  go_node_ = go_lib;

  // Dependency fixing
  AdditionalDependencies(file,
                         "proto_go_dep",
                         input().default_go_proto(),
                         go_node_);
}

bool ProtoLibraryNode::IncludeChildDependency(DependencyCollectionType type,
                                              LanguageType lang,
                                              Node* node) const {
  if (node == gen_node_) {
    return type == DEPENDENCY_FILES;
  }
  if (lang == CPP || lang == C_LANG) {
    return node == cc_node_;
  }
  if (lang == JAVA) {
    return node == java_node_;
  }
  if (lang == PYTHON) {
    return node == py_node_;
  }
  if (lang == GOLANG) {
    return node == go_node_;
  }
  return true;
}

void ProtoLibraryNode::PostParse() {
  // Figure out where protoc lives.
  string protoc = "protoc";  // use system binary by default.
  ResourceFileSet dep_binaries;
  Binaries(NO_LANG, &dep_binaries);
  for (const Resource& r : dep_binaries.files()) {
    if (r.basename() == "protoc") {
      protoc = "$(ROOT_DIR)/" + r.path();
      break;
    }
  }
  gen_node_->AddLocalEnvVariable(kProtocVar, protoc);
}

void ProtoLibraryNode::LocalWriteMake(Makefile* out) const {
  WriteBaseUserTarget(out);
}

}  // namespace repobuild
