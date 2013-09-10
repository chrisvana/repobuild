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
#include "repobuild/nodes/proto_library.h"
#include "repobuild/reader/buildfile.h"

using std::vector;
using std::string;
using std::set;

namespace repobuild {

void ProtoLibraryNode::Parse(BuildFile* file, const BuildFileNode& input) {
  // TODO(cvanarsdale): This function is ugly.

  Node::Parse(file, input);

  // Generate the output files.
  GenShNode* gen = new GenShNode(target().GetParallelTarget(file->NextName()),
                                 Node::input());
  for (const TargetInfo* dep : dependencies()) {
    gen->AddDependency(*dep);
  }
  gen->SetCd(false);
  AddSubNode(gen);
  string build_cmd, clean_cmd;
  vector<string> outputs;

  // And build them into a cc library
  CCLibraryNode* cc_lib = new CCLibraryNode(
      target().GetParallelTarget(file->NextName()),
      Node::input());
  AddSubNode(cc_lib);
  cc_lib->AddDependency(gen->target());
  vector<Resource> sources, headers;

  vector<Resource> input_files;
  current_reader()->ParseRepeatedFiles("sources", &input_files);
  if (input_files.empty()) {
    LOG(FATAL) << "proto_library requires input .proto files: "
               << target().full_path();
  }

  build_cmd = "protoc --cpp_out=";
  build_cmd += Node::input().genfile_dir();
  clean_cmd = "rm -f";
  for (const Resource& input_file : input_files) {
    string file = input_file.path();

    // Get just the relative path from this BUILD file.
    CHECK(strings::HasPrefix(file, target().dir()));

    if (!strings::HasSuffix(file, ".proto") &&
        !strings::HasSuffix(file, ".protodevel")) {
      LOG(FATAL) << "Expected .proto suffix: "
                 << file
                 << " (from target " << target().full_path() << ").";
    }

    string prefix = file.substr(0, file.size() - 6);  // strip .proto.

    build_cmd += " " + file;

    // c++
    string cpp_file = prefix + ".pb.cc";
    string hpp_file = prefix + ".pb.h";
    string cpp_full = strings::JoinPath(Node::input().genfile_dir(), cpp_file);
    string hpp_full = strings::JoinPath(Node::input().genfile_dir(), hpp_file);

    clean_cmd += " " + cpp_full;
    clean_cmd += " " + hpp_full;

    // Relative to BUILD file:
    outputs.push_back(cpp_file.substr(target().dir().size() + 1));
    outputs.push_back(hpp_file.substr(target().dir().size() + 1));

    // Relative to root:
    sources.push_back(Resource::FromRootPath(cpp_full));
    headers.push_back(Resource::FromRootPath(hpp_full));
  }

  gen->Set(build_cmd, clean_cmd, input_files, outputs);

  vector<Resource> objects;
  vector<string> cc_compile_args, header_compile_args;  // dummies.
  cc_lib->Set(sources, headers, objects, cc_compile_args, header_compile_args);
}

}  // namespace repobuild
