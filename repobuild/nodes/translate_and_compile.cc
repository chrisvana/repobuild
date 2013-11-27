// Copyright 2013
// Author: Christopher Van Arsdale
// Modified by Mark Vandevoorde

#include <string>
#include <set>
#include <iterator>
#include <vector>
#include "common/log/log.h"
#include "common/strings/path.h"
#include "common/strings/strutil.h"
#include "repobuild/env/input.h"
#include "repobuild/nodes/cc_library.h"
#include "repobuild/nodes/gen_sh.h"
#include "repobuild/nodes/go_library.h"
#include "repobuild/nodes/java_library.h"
#include "repobuild/nodes/translate_and_compile.h"
#include "repobuild/nodes/py_library.h"
#include "repobuild/reader/buildfile.h"

using std::vector;
using std::string;
using std::set;

namespace repobuild {
namespace {
const char kTranslatorVar[] = "TRANSLATOR";
const char kTranslatorOutputVar[] = "TRANSLATOR_OUTPUT";
}

void TranslateAndCompileNode::Parse(BuildFile* file,
				    const BuildFileNode& input) {
  Node::Parse(file, input);

  // Read the input files.
  vector<Resource> input_files;
  current_reader()->ParseRepeatedFiles("sources", &input_files);
  if (input_files.empty()) {
    LOG(FATAL) << "translate_and_compile requires input files: "
               << target().full_path();
  }
  vector<Resource> input_prefixes;
  FindPrefixes(input_files, &input_prefixes);

  // Figure out where translator lives.
  current_reader()->ParseStringField("translator", &translator_);
  if (translator_.empty()) {
    LOG(FATAL) << "translate_and_compile needs a translator field.  "
	       << "Target is: " << target().full_path();
  }

  // We will generate the files using translator, using a gen_sh
  // node. E.g: translator --cpp_out=.gen-files --java_out=.gen-files
  //   --go_out=.gen-files -I.  -I.gen-files -I.gen-src
  //   -I.gen-src/.gen-files testdata/a/a.proto
  gen_node_ = NewSubNodeWithCurrentDeps<GenShNode>(file);
  gen_node_->SetCd(false);
  gen_node_->SetMakeName(translator_);

  string translator_binary = "$" + string(kTranslatorVar);

  // Figure out which languages to generates.
  bool generate_cc = false;
  bool generate_java = false;
  bool generate_python = false;
  bool generate_go = false;
  current_reader()->ParseBoolField("generate_cc", &generate_cc);
  current_reader()->ParseBoolField("generate_java", &generate_java);
  current_reader()->ParseBoolField("generate_python", &generate_python);
  current_reader()->ParseBoolField("generate_go", &generate_go);

  // Find all of the output files, and build up our translator command line.
  string build_cmd = translator_binary;
  vector<Resource> outputs;

  bool has_language = false;

  // c++
  if (generate_cc) {
    has_language = true;
    vector<string> cc_translator_args;
    current_reader()->ParseRepeatedString("cc.translator_args",
					  &cc_translator_args);
    build_cmd += " " + strings::JoinAll(cc_translator_args, " ");
    GenerateCpp(input_prefixes, &outputs, file);
  }

  // java
  if (generate_java) {
    has_language = true;
    vector<string> java_translator_args;
    current_reader()->ParseRepeatedString("java.translator_args",
					  &java_translator_args);
    build_cmd += " " + strings::JoinAll(java_translator_args, " ");
    vector<string> java_classnames;
    current_reader()->ParseRepeatedString("java_classnames", &java_classnames);
    GenerateJava(file, input, input_prefixes, java_classnames, &outputs);
  }

  // python
  if (generate_python) {
    has_language = true;
    vector<string> py_translator_args;
    current_reader()->ParseRepeatedString("py.translator_args",
					  &py_translator_args);
    build_cmd += " " + strings::JoinAll(py_translator_args, " ");
    GeneratePython(input_prefixes, &outputs, file);
  }

  // go
  if (generate_go) {
    has_language = true;
    vector<string> go_translator_args;
    current_reader()->ParseRepeatedString("go.translator_args",
					  &go_translator_args);
    build_cmd += " " + strings::JoinAll(go_translator_args, " ");
    GenerateGo(input_prefixes, &outputs, file);
  }

  if (!has_language) {
    LOG(FATAL) << "translate_and_compile needs at least one \"generate_x\""
	       << " language. Target is: " << target().full_path();
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

void TranslateAndCompileNode::FindPrefixes(
    const vector<Resource>& input_files,
    vector<Resource>* prefixes) const {
  // Find all of the input basenames.
  for (const Resource& input_file : input_files) {
    const string& file = input_file.path();

    // Make sure the file is within our current directory.
    if (!strings::HasPrefix(file, target().dir())) {
      LOG(FATAL) << "translate_and_compile requires input "
                 << "files exist in this directory tree: "
                 << file << " vs " << target().dir();
    }

    prefixes->push_back(Resource::FromLocalPath(
        input_file.dirname(),
        input_file.basename().substr(0, input_file.basename().rfind('.'))));
  }
}

void TranslateAndCompileNode::AdditionalDependencies(
    BuildFile* file,
    const string& dep_field,
    const string& default_dep_value,
    Node* node) {
  string dep;
  if (!current_reader()->ParseStringField(dep_field, false, &dep)) {
    dep = default_dep_value;
  }

  // NB: This is sadly trickier than it should be. The script node
  // that runs translator depends on everything (so it can make sure
  // translator is properly built for each langauge). However, we
  // don't want everyone who depends on translate_and_compile to have
  // to depend on every langauge (e.g. cc_library should only depend
  // on cc_node_, etc). That is all covered by IncludeChildDependency
  // below.
  TargetInfo target = file->ComputeTargetInfo(dep);
  node->AddDependencyTarget(gen_node_->target());
  gen_node_->AddDependencyTarget(target);
  node->AddDependencyTarget(target);
}

void TranslateAndCompileNode::GenerateCpp(
    const vector<Resource>& input_prefixes,
    vector<Resource>* outputs,
    BuildFile* file) {
  vector<Resource> cc_sources, cc_headers;
  vector<string> source_suffixes, header_suffixes;
  current_reader()->ParseRepeatedString("cc.source_suffixes", &source_suffixes);
  current_reader()->ParseRepeatedString("cc.header_suffixes", &header_suffixes);
  
  for (const Resource& prefix : input_prefixes) {
    for (const string& suffix : source_suffixes) {
      string cpp_file = prefix.path() + suffix;
      cc_sources.push_back(Resource::FromLocalPath(input().genfile_dir(),
						   cpp_file));
    }
    for (const string& suffix : header_suffixes) {
      string hpp_file = prefix.path() + suffix;
      cc_headers.push_back(Resource::FromLocalPath(input().genfile_dir(),
						   hpp_file));
    }
  }

  outputs->insert(outputs->end(), cc_sources.begin(), cc_sources.end());
  outputs->insert(outputs->end(), cc_headers.begin(), cc_headers.end());

  cc_node_ = NewSubNode<CCLibraryNode>(file);

  // dummies:
  vector<Resource> objects;
  vector<string> cc_compile_args, header_compile_args;
  cc_node_->Set(cc_sources, cc_headers, objects,
                cc_compile_args, header_compile_args);

  // Dependency fixing
  string support_library;
  current_reader()->ParseStringField("cc.support_library", &support_library);
  AdditionalDependencies(file, "proto_cc_dep", support_library, cc_node_);
}

void TranslateAndCompileNode::GenerateJava(BuildFile* file,
					     const BuildFileNode& input,
                                     const vector<Resource>& input_prefixes,
                                     const vector<string>& java_classnames,
                                     vector<Resource>* outputs) {
  vector<Resource> java_sources;
  if (java_classnames.size() > 0 &&
      java_classnames.size() != input_prefixes.size()) {
    LOG(FATAL) << "java_classnames must match up 1:1 with proto source files: "
               << target().full_path();
  }

  for (int i = 0; i < input_prefixes.size(); ++i) {
    const Resource& prefix = input_prefixes[i];

    // Java classnames are trickier.
    string java_classname;
    if (!java_classnames.empty()) {
      java_classname = java_classnames[i];
    } else {
      vector<string> splits = strings::SplitString(prefix.basename(), "_");
      for (const string& split : splits) {
        java_classname += strings::Capitalize(split);
      }
    }

    string java_basename = java_classname + ".java";
    java_sources.push_back(Resource::FromLocalPath(
        Node::input().genfile_dir(),
        strings::JoinPath(prefix.dirname(), java_basename)));
  }
  outputs->insert(outputs->end(), java_sources.begin(), java_sources.end());

  java_node_ = NewSubNode<JavaLibraryNode>(file);
  java_node_->Set(file, input, java_sources);

  // Dependency fixing
  string support_library;
  current_reader()->ParseStringField("java.support_library", &support_library);
  AdditionalDependencies(file, "proto_java_dep", support_library, java_node_);
}

void TranslateAndCompileNode::GeneratePython(
    const vector<Resource>& input_prefixes,
    vector<Resource>* outputs,
    BuildFile* file) {
  vector<Resource> python_sources;

  for (const Resource& prefix : input_prefixes) {
    // TODO(cvanarsdale): This will sadly break some day.
    string python_file = prefix.path() + "_pb2.py";
    python_sources.push_back(Resource::FromLocalPath(
        input().genfile_dir(), python_file));
  }
  outputs->insert(outputs->end(), python_sources.begin(), python_sources.end());

  py_node_ = NewSubNode<PyLibraryNode>(file);
  py_node_->Set(python_sources);

  // Dependency fixing
  string support_library;
  current_reader()->ParseStringField("py.support_library", &support_library);
  AdditionalDependencies(file, "proto_py_dep", support_library, py_node_);
}

void TranslateAndCompileNode::GenerateGo(const vector<Resource>& input_prefixes,
					 vector<Resource>* outputs,
					 BuildFile* file) {
  vector<Resource> go_sources;

  for (const Resource& prefix : input_prefixes) {
    string go_file = prefix.path() + ".pb.go";
    go_sources.push_back(Resource::FromLocalPath(
        input().genfile_dir(), go_file));
  }
  outputs->insert(outputs->end(), go_sources.begin(), go_sources.end());

  go_node_ = NewSubNode<GoLibraryNode>(file);
  go_node_->Set(go_sources);

  // Dependency fixing
  string support_library;
  current_reader()->ParseStringField("go.support_library", &support_library);
  AdditionalDependencies(file, "proto_go_dep", support_library, go_node_);
}

bool TranslateAndCompileNode::IncludeChildDependency(
    DependencyCollectionType type,
    LanguageType lang,
    Node* node) const {
  // TODO(cvanarsdale): Some sort of "Mux node" or "conditional dependency node"
  // instead of this.
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
  if (lang == GO_LANG) {
    return node == go_node_;
  }
  return true;
}

void TranslateAndCompileNode::PostParse() {
  Node::PostParse();

  // Figure out where translator lives.
  string translator = "translator";  // use system binary by default.
  ResourceFileSet dep_binaries;
  Binaries(NO_LANG, &dep_binaries);
  for (const Resource& r : dep_binaries.files()) {
    if (r.basename() == translator_) {
      translator = "$(ROOT_DIR)/" + r.path();
      break;
    }
  }
  gen_node_->AddLocalEnvVariable(kTranslatorVar, translator);

  // Tell translator where to put generated source
  gen_node_->AddLocalEnvVariable(kTranslatorOutputVar,
				 Node::input().genfile_dir());
}

void TranslateAndCompileNode::LocalWriteMake(Makefile* out) const {
  WriteBaseUserTarget(out);
}

}  // namespace repobuild
