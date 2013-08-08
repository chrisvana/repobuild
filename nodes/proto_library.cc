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
#include "nodes/cc_library.h"
#include "nodes/gen_sh.h"
#include "nodes/proto_library.h"
#include "repobuild/reader/buildfile.h"

using std::vector;
using std::string;
using std::set;

namespace repobuild {

void ProtoLibraryNode::Parse(BuildFile* file, const BuildFileNode& input) {
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
  vector<string> inputs, outputs;

  // And build them into a cc library
  CCLibraryNode* cc_lib = new CCLibraryNode(
      target().GetParallelTarget(file->NextName()),
      Node::input());
  AddSubNode(cc_lib);
  cc_lib->AddDependency(gen->target());
  vector<string> sources, headers, objects, cc_compile_args;

  vector<string> input_files;
  ParseRepeatedFiles(input, "sources", &input_files);
  if (input_files.empty()) {
    LOG(FATAL) << "proto_library requires input .proto files: "
               << target().full_path();
  }

  build_cmd = "protoc --cpp_out=";
  build_cmd += Node::input().genfile_dir();
  clean_cmd = "rm -f";
  for (const string& file : input_files) {
    // Get just the relative path from this BUILD file.
    CHECK(strings::HasPrefix(file, target().dir()));

    if (!strings::HasSuffix(file, ".proto")) {
      LOG(FATAL) << "Expected .proto suffix: "
                 << file
                 << " (from target " << target().full_path() << ").";
    }

    string prefix = file.substr(0, file.size() - 6);  // strip .proto.

    build_cmd += " " + file;

    // c++
    string cpp_file = prefix + ".pb.cc";
    string hpp_file = prefix + ".pb.h";

    clean_cmd += " $(GEN_DIR)/" + cpp_file;
    clean_cmd += " $(GEN_DIR)/" + hpp_file;

    outputs.push_back(cpp_file);
    outputs.push_back(hpp_file);

    sources.push_back(strings::JoinPath(Node::input().genfile_dir(), cpp_file));
    headers.push_back(strings::JoinPath(Node::input().genfile_dir(), hpp_file));
  }

  gen->Set(build_cmd, clean_cmd, inputs, outputs);
  cc_lib->Set(sources, headers, objects, cc_compile_args);
}

}  // namespace repobuild
