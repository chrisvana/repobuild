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

  // cc_compile_args
  ParseRepeatedString(input, "cc_compile_args", &cc_compile_args_);

  // header_compile_args
  ParseRepeatedString(input, "header_compile_args", &header_compile_args_);

  // cc_linker_args
  ParseRepeatedString(input, "cc_linker_args", &cc_linker_args_);
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
}

void CCLibraryNode::WriteMakefile(const vector<const Node*>& all_deps,
                                  string* out) const {
  WriteMakefileInternal(all_deps, true, out);
}

void CCLibraryNode::WriteMakefileInternal(const vector<const Node*>& all_deps,
                                          bool should_write_target,
                                          string* out) const {
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
    out->append(target().make_path());
    out->append(":");
    for (const string& source : sources_) {
      out->append(" ");
      out->append(ObjForSource(source));
    }
    out->append("\n\n.PHONY: ");
    out->append(target().make_path());
    out->append("\n\n");
  }
}

void CCLibraryNode::WriteCompile(const string& source,
                                 const set<string>& input_files,
                                 const vector<const Node*>& all_deps,
                                 string* out) const {
  string obj = ObjForSource(source);
  out->append(obj + ":");

  // Dependencies.
  for (const string& input : input_files) {
    out->append(" ");
    out->append(input);
  }
  out->append(" ");
  out->append(source);

  // Mkdir command.
  out->append("\n\t@");  // silent
  out->append("mkdir -p ");
  out->append(strings::PathDirname(obj));

    // Compile command.
  out->append("\n\t");
  out->append(DefaultCompileFlags(strings::HasSuffix(source, ".cc") ||
                                  strings::HasSuffix(source, ".cpp")));
  out->append(" -I");
  out->append(input().root_dir());
  out->append(" -I");
  out->append(input().genfile_dir());
  out->append(" -I");
  out->append(input().source_dir());
  out->append(" -I");
  out->append(strings::JoinPath(input().source_dir(), input().genfile_dir()));

  set<string> header_compile_args;
  CollectCompileFlags(all_deps, &header_compile_args);
  for (const string flag : header_compile_args) {
    out->append(" ");
    out->append(flag);
  }
  for (const string flag : cc_compile_args_) {
    out->append(" ");
    out->append(flag);
  }
  out->append(" ");
  out->append(source);

  // Output object.
  out->append(" -o ");
  out->append(obj);

  out->append("\n\n");
}

void CCLibraryNode::DependencyFiles(vector<string>* files) const {
  Node::DependencyFiles(files);
  for (int i = 0; i < headers_.size(); ++i) {
    files->push_back(headers_[i]);
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
  for (const string flag : cc_linker_args_) {
    flags->insert(flag);
  }
}

void CCLibraryNode::CompileFlags(std::set<std::string>* flags) const {
  Node::CompileFlags(flags);
  for (const string flag : header_compile_args_) {
    flags->insert(flag);
  }
}

std::string CCLibraryNode::DefaultCompileFlags(bool cpp_mode) const {
  return (cpp_mode ? "$(COMPILE.cc)" : "$(COMPILE.c)");
}

namespace {
// TODO(cvanarsdale): This is clunky. Nicer to have flags in a registered list
// with conditionals already set.
bool IsGccFlag(const string& flag) {
  return (!strings::HasPrefix(flag, "-stdlib") &&
          (flag == "-Q" || !strings::HasPrefix(flag, "-Q")));
}
string JoinFlags(const vector<string>& flags, bool gcc_only) {
  string out;
  for (const string& flag : flags) {
    if (!gcc_only || IsGccFlag(flag)) {
      out.append(" ");
      out.append(flag);
    }
  }
  return out;
}

string WriteLdflag(const Input& input, bool gcc) {
  string out = "LDFLAGS=";
  out.append(JoinFlags(input.flags("-L"), gcc));
  out.append("\n");
  return out;
}

string WriteCflag(const Input& input, bool gcc) {
  string out = "CFLAGS=";
  out.append(JoinFlags(input.flags("-C"), gcc));
  out.append("\n");
  return out;
}

string WriteCxxflag(const Input& input, bool gcc) {
  string out = "CXXFLAGS=";
  out.append(JoinFlags(input.flags("-C"), gcc));
  out.append(JoinFlags(input.flags("-X"), gcc));
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
              "egrep '(^gcc|^g\\+\\+)' | head -n 1 | wc -l)\n");

  // Write the global values
  // CFLAGS:
  out->append("ifeq ($(CC_GCC),1)\n");
  out->append("\t" + WriteCflag(input, true));
  out->append("else\n");
  out->append("\t" + WriteCflag(input, false));
  out->append("endif\n");

  // CXXFLAGS and LDFLAGS
  out->append("ifeq ($(CXX_GCC),1)\n");
  out->append("\t" + WriteLdflag(input, true));
  out->append("\t" + WriteCxxflag(input, true));
  out->append("else\n");
  out->append("\t" + WriteLdflag(input, false));
  out->append("\t" + WriteCxxflag(input, false));
  out->append("endif\n\n");
}

string CCLibraryNode::ObjForSource(const std::string& source) const {
  return strings::JoinPath(input().object_dir(), source + ".o");
}

}  // namespace repobuild
