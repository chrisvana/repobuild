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
#include "repobuild/reader/buildfile.h"

using std::vector;
using std::string;
using std::set;

namespace repobuild {

void CCLibraryNode::Parse(BuildFile* file, const BuildFileNode& input) {
  Node::Parse(file, input);

  // cc_sources
  ParseRepeatedFiles(input, "cc_sources", &sources_);

  // cc_headers
  ParseRepeatedFiles(input, "cc_headers", &headers_);

  // cc_objs
  ParseRepeatedFiles(input, "cc_objects", &objects_);

  // TODO(cvanarsdale): sources/headers/objs in the conditional.


  // cc_compile_args, header_compile_args, cc_linker_args
  ParseRepeatedString(input, "cc_compile_args", &cc_compile_args_);
  ParseRepeatedString(input, "header_compile_args", &header_compile_args_);
  ParseRepeatedString(input, "cc_linker_args", &cc_linker_args_);

  // gcc
  ParseRepeatedString(input, "gcc.cc_compile_args", &gcc_cc_compile_args_);
  ParseRepeatedString(input, "gcc.header_compile_args",
                      &gcc_header_compile_args_);
  ParseRepeatedString(input, "gcc.cc_linker_args", &gcc_cc_linker_args_);

  // clang
  ParseRepeatedString(input, "clang.cc_compile_args", &clang_cc_compile_args_);
  ParseRepeatedString(input, "clang.header_compile_args",
                      &clang_header_compile_args_);
  ParseRepeatedString(input, "clang.cc_linker_args", &clang_cc_linker_args_);

  Init();
}

void CCLibraryNode::Set(const vector<string>& sources,
                        const vector<string>& headers,
                        const vector<string>& objects,
                        const vector<string>& cc_compile_args,
                        const vector<string>& header_compile_args) {
  sources_ = sources;
  headers_ = headers;
  objects_ = objects;
  cc_compile_args_ = cc_compile_args;
  header_compile_args_ = cc_compile_args;
  Init();
}

void CCLibraryNode::Init() {
  if (!headers_.empty()) {
    MutableVariable("headers")->SetValue(strings::Join(headers_, " "));
  }

  // cc_compile_args
  AddVariable("cxx_compile_args", "c_compile_args",
              strings::JoinWith(
                  " ",
                  strings::Join(cc_compile_args_, " "),
                  strings::Join(gcc_cc_compile_args_, " ")),
              strings::JoinWith(
                  " ",
                  strings::Join(cc_compile_args_, " "),
                  strings::Join(clang_cc_compile_args_, " ")));
  
  // header_compile_args
  AddVariable("cxx_header_compile_args", "c_header_compile_args",
              strings::JoinWith(
                  " ",
                  strings::Join(header_compile_args_, " "),
                  strings::Join(gcc_header_compile_args_, " ")),
              strings::JoinWith(
                  " ",
                  strings::Join(header_compile_args_, " "),
                  strings::Join(clang_header_compile_args_, " ")));

  // cc_linker_args
  AddVariable("cc_linker_args", "cc_linker_args",  // NB: no distinction.
              strings::JoinWith(
                  " ",
                  strings::Join(cc_linker_args_, " "),
                  strings::Join(gcc_cc_linker_args_, " ")),
              strings::JoinWith(
                  " ",
                  strings::Join(cc_linker_args_, " "),
                  strings::Join(gcc_cc_linker_args_, " ")));
}

void CCLibraryNode::WriteMakefile(const vector<const Node*>& all_deps,
                                  string* out) const {
  WriteMakefileInternal(all_deps, true, out);
}

void CCLibraryNode::WriteMakefileInternal(const vector<const Node*>& all_deps,
                                          bool should_write_target,
                                          string* out) const {
  // Write header variable
  GetVariable("headers").WriteMake(out);

  // Figure out the set of input files.
  set<string> input_files;
  CollectDependencies(all_deps, &input_files);

  // Now write phases, one per .cc
  for (int i = 0; i < sources_.size(); ++i) {
    // Output object.
    WriteCompile(sources_[i], input_files, all_deps, out);
  }

  // Now write user target
  if (should_write_target) {
    set<string> targets;
    for (const string& source : sources_) {
      targets.insert(ObjForSource(source));
    }
    out->append(WriteBaseUserTarget(targets));
  }
}

void CCLibraryNode::WriteCompile(const string& source,
                                 const set<string>& input_files,
                                 const vector<const Node*>& all_deps,
                                 string* out) const {
  string obj = ObjForSource(source);
  out->append(obj + ": ");

  // Dependencies.
  out->append(strings::Join(input_files, " "));
  out->append(" ");
  out->append(source);

  // Mkdir command.
  out->append("\n\t@");  // silent
  out->append("mkdir -p ");
  out->append(strings::PathDirname(obj));

    // Compile command.
  out->append("\n\t");
  out->append("@echo Compiling: ");
  out->append(source);
  out->append("\n\t@");
  bool cpp = (strings::HasSuffix(source, ".cc") ||
              strings::HasSuffix(source, ".cpp"));
  out->append(DefaultCompileFlags(cpp));
  out->append(" ");

  // Include directories
  out->append(strings::JoinWith(
      " ",
      "-I" + input().root_dir(),
      "-I" + input().genfile_dir(),
      "-I" + input().source_dir(),
      "-I" + strings::JoinPath(input().source_dir(), input().genfile_dir())));
  out->append(" ");

  // Output compile args
  set<string> header_compile_args;
  CollectCompileFlags(cpp, all_deps, &header_compile_args);
  out->append(strings::JoinWith(
      " ",
      strings::Join(header_compile_args, " "),
      GetVariable(cpp ? "cxx_compile_args" : "c_compile_args").ref_name()));

  // Output source file
  out->append(" " + source);

  // Output object.
  out->append(" -o " + obj);

  out->append("\n\n");
}

void CCLibraryNode::DependencyFiles(vector<string>* files) const {
  Node::DependencyFiles(files);
  if (HasVariable("headers")) {
    files->push_back(GetVariable("headers").ref_name());
  }
}

void CCLibraryNode::ObjectFiles(vector<string>* files) const {
  Node::ObjectFiles(files);
  for (int i = 0; i < sources_.size(); ++i) {
    files->push_back(strings::JoinPath(input().object_dir(),
                                       sources_[i] + ".o"));
  }
  for (const string& obj : objects_) {
    files->push_back(obj);
  }
}

void CCLibraryNode::LinkFlags(std::set<std::string>* flags) const {
  Node::LinkFlags(flags);
  if (HasVariable("cc_linker_args")) {
    flags->insert(GetVariable("cc_linker_args").ref_name());
  }
}

void CCLibraryNode::CompileFlags(bool cxx, std::set<std::string>* flags) const {
  Node::CompileFlags(cxx, flags);
  if (cxx) {
    if (HasVariable("cxx_header_compile_args")) {
      flags->insert(GetVariable("cxx_header_compile_args").ref_name());
    }
  } else {
    if (HasVariable("c_header_compile_args")) {
      flags->insert(GetVariable("c_header_compile_args").ref_name());
    }
  }
}

std::string CCLibraryNode::DefaultCompileFlags(bool cpp_mode) const {
  return (cpp_mode ? "$(COMPILE.cc)" : "$(COMPILE.c)");
}

namespace {

bool IsBasicFlag(const string& flag) {
  return (strings::HasPrefix(flag, "-stdlib") ||
          strings::HasPrefix(flag, "-std") ||  // redundant, but for clarity.
          strings::HasPrefix(flag, "-pthread"));
}

string JoinFlags(const vector<string>& flags,
                 bool gcc_only,
                 bool basic_only) {
  string out;
  for (const string& flag_input : flags) {
    // Figure out the flag name and the compiler.
    bool gcc = false, clang = false;
    string flag;
    if (strings::HasPrefix(flag_input, "gcc=")) {
      flag = flag_input.substr(4);
      gcc = true;
    } else if (strings::HasPrefix(flag_input, "clang=")) {
      flag = flag_input.substr(6);
      clang = true;
    } else {
      gcc = clang = true;
      flag = flag_input;
    }

    // Wrong compiler.
    if ((gcc_only && !gcc) || (!gcc_only && !clang)) {
      continue;
    }

    // Check basic/full
    if (basic_only && !IsBasicFlag(flag)) {
      continue;
    }

    out.append(" ");
    out.append(flag);
  }
  return out;
}

string WriteLdflag(const Input& input, bool gcc) {
  string out = "LDFLAGS=";
  out.append(JoinFlags(input.flags("-L"), gcc, false));
  out.append("\n");
  return out;
}

string WriteCflag(const Input& input, bool gcc, bool basic) {
  string out = (basic ? "BASIC_CFLAGS=" : "CFLAGS=");
  out.append(JoinFlags(input.flags("-C"), gcc, basic));
  out.append("\n");
  return out;
}

string WriteCxxflag(const Input& input, bool gcc, bool basic) {
  string out = (basic ? "BASIC_CXXFLAGS=" : "CXXFLAGS=");
  out.append(JoinFlags(input.flags("-C"), gcc, basic));
  out.append(JoinFlags(input.flags("-X"), gcc, basic));
  out.append("\n");
  return out;
}
                 
}  // anonymous namespace

// static
void CCLibraryNode::WriteMakeHead(const Input& input, string* out) {
  // Some conditional variables
  out->append("# Some compiler specific flag settings.\n");
  out->append("CXX_GCC := $(shell $(CXX) --version | "
             "egrep '(^gcc|^g\\+\\+)' | head -n 1 | wc -l)\n");
  out->append("CC_GCC := $(shell $(CC) --version | "
              "egrep '(^gcc|^g\\+\\+|^cc)' | head -n 1 | wc -l)\n");

  // Write the global values
  // CFLAGS:
  out->append("ifeq ($(CC_GCC),1)\n");
  out->append("\t" + WriteCflag(input, true, false));
  out->append("\t" + WriteCflag(input, true, true));
  out->append("else\n");
  out->append("\t" + WriteCflag(input, false, false));
  out->append("\t" + WriteCflag(input, false, true));
  out->append("endif\n");

  // CXXFLAGS and LDFLAGS
  out->append("ifeq ($(CXX_GCC),1)\n");
  out->append("\t" + WriteLdflag(input, true));
  out->append("\t" + WriteCxxflag(input, true, false));
  out->append("\t" + WriteCxxflag(input, true, true));
  out->append("else\n");
  out->append("\t" + WriteLdflag(input, false));
  out->append("\t" + WriteCxxflag(input, false, false));
  out->append("\t" + WriteCxxflag(input, false, true));
  out->append("endif\n\n");
}

string CCLibraryNode::ObjForSource(const std::string& source) const {
  return strings::JoinPath(input().object_dir(), source + ".o");
}

void CCLibraryNode::AddVariable(const string& cpp_name,
                                const string& c_name,
                                const string& gcc_value,
                                const string& clang_value) {
  if (gcc_value == clang_value) {
    if (!gcc_value.empty()) {
      MutableVariable(c_name)->SetValue(gcc_value);
      MutableVariable(cpp_name)->SetValue(gcc_value);
    }
  } else {
    MutableVariable(c_name)->SetCondition(
        "CC_GCC", gcc_value, clang_value);
    MutableVariable(cpp_name)->SetCondition(
        "CXX_GCC", gcc_value, clang_value);
  }
}


}  // namespace repobuild
