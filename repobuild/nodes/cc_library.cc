// Copyright 2013
// Author: Christopher Van Arsdale

#include <string>
#include <set>
#include <iterator>
#include <vector>
#include "common/log/log.h"
#include "common/strings/path.h"
#include "common/strings/strutil.h"
#include "repobuild/env/input.h"
#include "repobuild/nodes/cc_library.h"
#include "repobuild/reader/buildfile.h"

using std::vector;
using std::string;
using std::set;

namespace repobuild {
namespace {
const char kHeaderVariable[] = "headers";
const char kLinkerVariable[] = "cc_linker_args";
const char kCCompileArgs[] = "c_compile_args";
const char kCxxCompileArgs[] = "cxx_compile_args";
const char kCHeaderArgs[] = "c_header_compile_args";
const char kCxxHeaderArgs[] = "cxx_header_compile_args";
const char kCGcc[] = "CC_GCC";
const char kCxxGcc[] = "CXX_GCC";
}

void CCLibraryNode::Parse(BuildFile* file, const BuildFileNode& input) {
  Node::Parse(file, input);

  // cc_sources
  current_reader()->ParseRepeatedFiles("cc_sources", &sources_);

  // cc_headers
  current_reader()->ParseRepeatedFiles("cc_headers", &headers_);

  // cc_objs
  current_reader()->ParseRepeatedFiles("cc_objects", &objects_);

  // alwayslink
  bool alwayslink = false;
  if (current_reader()->ParseBoolField("alwayslink", &alwayslink) &&
      alwayslink) {
    for (Resource& r : objects_) {
      r.add_tag("alwayslink");
    }
    for (Resource& r : sources_) {
      r.add_tag("alwayslink");
    }
  }

  // cc_include_dirs
  vector<Resource> cc_include_dirs;
  current_reader()->ParseRepeatedFiles("cc_include_dirs",
                                       false,  // directory need not exist.
                                       &cc_include_dirs);
  for (const Resource& r : cc_include_dirs) {
    cc_include_dirs_.push_back(r.path());
  }

  // cc_compile_args, header_compile_args, cc_linker_args
  current_reader()->ParseRepeatedString("cc_compile_args",
                                        &cc_compile_args_);
  current_reader()->ParseRepeatedString("header_compile_args",
                                        &header_compile_args_);
  current_reader()->ParseRepeatedString("cc_linker_args",
                                        &cc_linker_args_);

  LG << "HERE: " << target().full_path();
  LG << header_compile_args_.size();
  for (const string& str: header_compile_args_) {
    LG << str;
  }

  // gcc
  current_reader()->ParseRepeatedString("gcc.cc_compile_args",
                                        &gcc_cc_compile_args_);
  current_reader()->ParseRepeatedString("gcc.header_compile_args",
                                        &gcc_header_compile_args_);
  current_reader()->ParseRepeatedString("gcc.cc_linker_args",
                                        &gcc_cc_linker_args_);

  // clang
  current_reader()->ParseRepeatedString("clang.cc_compile_args",
                                        &clang_cc_compile_args_);
  current_reader()->ParseRepeatedString("clang.header_compile_args",
                                        &clang_header_compile_args_);
  current_reader()->ParseRepeatedString("clang.cc_linker_args",
                                        &clang_cc_linker_args_);

  Init();
}

void CCLibraryNode::Set(const vector<Resource>& sources,
                        const vector<Resource>& headers,
                        const vector<Resource>& objects,
                        const vector<string>& cc_compile_args,
                        const vector<string>& header_compile_args) {
  sources_ = sources;
  headers_ = headers;
  objects_ = objects;
  cc_compile_args_ = cc_compile_args;
  header_compile_args_ = cc_compile_args;
  Init();
}

void CCLibraryNode::PreInitSources(const ResourceFileSet& sources,
                                   const ResourceFileSet& headers) {
  for (const Resource& r : sources) {
    sources_.push_back(r);
  }
  for (const Resource& r : headers) {
    headers_.push_back(r);
  }
}

void CCLibraryNode::Init() {
  if (!headers_.empty()) {
    MutableVariable(kHeaderVariable)->SetValue(strings::JoinAll(headers_, " "));
  }

  // cc_compile_args
  AddVariable(kCxxCompileArgs, kCCompileArgs,
              strings::JoinWith(
                  " ",
                  strings::JoinAll(cc_compile_args_, " "),
                  strings::JoinAll(gcc_cc_compile_args_, " ")),
              strings::JoinWith(
                  " ",
                  strings::JoinAll(cc_compile_args_, " "),
                  strings::JoinAll(clang_cc_compile_args_, " ")));
  
  // header_compile_args
  AddVariable(kCxxHeaderArgs, kCHeaderArgs,
              strings::JoinWith(
                  " ",
                  strings::JoinAll(header_compile_args_, " "),
                  strings::JoinAll(gcc_header_compile_args_, " ")),
              strings::JoinWith(
                  " ",
                  strings::JoinAll(header_compile_args_, " "),
                  strings::JoinAll(clang_header_compile_args_, " ")));

  // cc_linker_args
  AddVariable(kLinkerVariable, kLinkerVariable,  // NB: no distinction.
              strings::JoinWith(
                  " ",
                  strings::JoinAll(cc_linker_args_, " "),
                  strings::JoinAll(gcc_cc_linker_args_, " ")),
              strings::JoinWith(
                  " ",
                  strings::JoinAll(cc_linker_args_, " "),
                  strings::JoinAll(gcc_cc_linker_args_, " ")));
}

void CCLibraryNode::LocalWriteMake(Makefile* out) const {
  LocalWriteMakeInternal(true, out);
}

void CCLibraryNode::LocalWriteMakeInternal(bool should_write_target,
                                           Makefile* out) const {
  // Figure out the set of input files.
  ResourceFileSet input_files;
  InputDependencyFiles(CPP, &input_files);  // any object files/headers/etc.
  CCLibraryNode::LocalDependencyFiles(CPP, &input_files);  // our headers

  // Now write phases, one per .cc
  for (int i = 0; i < sources_.size(); ++i) {
    // Output object.
    WriteCompile(sources_[i], input_files, out);
  }

  // Now write user target (so users can type "make path/to/exec|lib").
  if (should_write_target) {
    ResourceFileSet targets;
    for (const Resource& source : sources_) {
      targets.Add(ObjForSource(source));
    }
    WriteBaseUserTarget(targets, out);
  }
}

void CCLibraryNode::WriteCompile(const Resource& source,
                                 const ResourceFileSet& input_files,
                                 Makefile* out) const {
  Resource obj = ObjForSource(source);

  // Rule=> obj: <input header files> source.cc
  Makefile::Rule* rule =
      out->StartRule(obj.path(),
                     strings::JoinWith(
                         " ",
                         strings::JoinAll(input_files.files(), " "),
                         source.path()));
  
  // Mkdir command.
  rule->WriteCommand("mkdir -p " + obj.dirname());

  // Compile command (.e.g $(COMPILE.c) or $(COMPILE.cc)).
  bool cpp = (strings::HasSuffix(source.basename(), ".cc") ||
              strings::HasSuffix(source.basename(), ".cpp"));
  string compile = DefaultCompileFlags(cpp);

  // Include directories.
  string include_dirs;
  {
    set<string> include_dir_set;
    IncludeDirs(cpp ? CPP : C_LANG, &include_dir_set);
    for (const string& str: include_dir_set) {
      include_dirs += (include_dirs.empty() ? "-I" : " -I") + str;
    }
  }

  // Compile args
  string output_compile_args;
  {
    set<string> header_compile_args;
    CompileFlags(cpp ? CPP : C_LANG, &header_compile_args);
    output_compile_args = strings::JoinWith(
        " ",
        strings::JoinAll(header_compile_args, " "),
        GetVariable(cpp ? kCxxCompileArgs : kCCompileArgs).ref_name());
  }

  // Actual make command.
  rule->WriteUserEcho("Compiling",
                      source.path() + " (" + (cpp ? "c++" : "c") + ")");
  rule->WriteCommand(strings::JoinWith(
      " ",
      compile,
      include_dirs,
      output_compile_args,
      source.path(),
      "-o " + obj.path()));

  out->FinishRule(rule);
}

void CCLibraryNode::LocalDependencyFiles(LanguageType lang,
                                         ResourceFileSet* files) const {
  if (HasVariable(kHeaderVariable)) {
    files->Add(Resource::FromRaw(
        GetVariable(kHeaderVariable).ref_name()));
  }
}

void CCLibraryNode::LocalObjectFiles(LanguageType lang,
                                     ResourceFileSet* files) const {
  for (const Resource& src : sources_) {
    files->Add(ObjForSource(src));
  }
  for (const Resource& obj : objects_) {
    files->Add(obj);
  }
}

void CCLibraryNode::LocalLinkFlags(LanguageType lang,
                                   set<string>* flags) const {
  if (lang == C_LANG || lang == CPP) {
    if (HasVariable(kLinkerVariable)) {
      flags->insert(GetVariable(kLinkerVariable).ref_name());
    }
  }
}

void CCLibraryNode::LocalIncludeDirs(LanguageType lang,
                                     set<string>* dirs) const {
  dirs->insert(cc_include_dirs_.begin(), cc_include_dirs_.end());
}

void CCLibraryNode::LocalCompileFlags(LanguageType lang,
                                      set<string>* flags) const {
  if (lang == CPP) {
    if (HasVariable(kCxxHeaderArgs)) {
      flags->insert(GetVariable(kCxxHeaderArgs).ref_name());
    }
  } else if (lang == C_LANG) {
    if (HasVariable(kCHeaderArgs)) {
      flags->insert(GetVariable(kCHeaderArgs).ref_name());
    }
  }
}

string CCLibraryNode::DefaultCompileFlags(bool cpp_mode) const {
  return (cpp_mode ? "$(COMPILE.cc)" : "$(COMPILE.c)");
}

namespace {

// TODO(cvanarsdale): Flag? Data file?
bool IsBasicFlag(const string& flag) {
  return (strings::HasPrefix(flag, "-stdlib") ||
          strings::HasPrefix(flag, "-std") ||  // redundant, but for clarity.
          strings::HasPrefix(flag, "-pthread") ||
          strings::HasPrefix(flag, "-Qunused-arguments"));
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
void CCLibraryNode::WriteMakeHead(const Input& input, Makefile* out) {
  // Some conditional variables
  out->append("# Some compiler specific flag settings.\n");
  out->append(string(kCxxGcc) + " := $(shell echo $$($(CXX) --version | "
              "egrep '(gcc|g\\+\\+)' | head -n 1 | wc -l))\n");
  out->append(string(kCGcc) + " := $(shell echo $$($(CC) --version | "
              "egrep '(gcc|g\\+\\+|^cc)' | head -n 1 | wc -l))\n");

  // Write the global values
  // CFLAGS:
  out->append("ifeq ($(" + string(kCGcc) + "),1)\n");
  out->append("\t" + WriteCflag(input, true, false));
  out->append("\t" + WriteCflag(input, true, true));
  out->append("else\n");
  out->append("\t" + WriteCflag(input, false, false));
  out->append("\t" + WriteCflag(input, false, true));
  out->append("endif\n");

  // CXXFLAGS and LDFLAGS
  out->append("ifeq ($(" + string(kCxxGcc) + "),1)\n");
  out->append("\tLD_FORCE_LINK_START := -Wl,--whole-archive\n");
  out->append("\tLD_FORCE_LINK_END := -Wl,--no-whole-archive\n");
  out->append("\t" + WriteLdflag(input, true));
  out->append("\t" + WriteCxxflag(input, true, false));
  out->append("\t" + WriteCxxflag(input, true, true));
  out->append("else\n");
  out->append("\tLD_FORCE_LINK_START := -Wl,-force_load\n");
  out->append("\tLD_FORCE_LINK_END := \n");
  out->append("\t" + WriteLdflag(input, false));
  out->append("\t" + WriteCxxflag(input, false, false));
  out->append("\t" + WriteCxxflag(input, false, true));
  out->append("endif\n\n");
}

Resource CCLibraryNode::ObjForSource(const Resource& source) const {
  Resource r = Resource::FromLocalPath(input().object_dir(),
                                       StripSpecialDirs(source.path()) + ".o");
  r.CopyTags(source);
  return r;
}

void CCLibraryNode::AddVariable(const string& cpp_name,
                                const string& c_name,
                                const string& gcc_value,
                                const string& clang_value) {
  AddConditionalVariable(cpp_name, kCxxGcc, gcc_value, clang_value);
  AddConditionalVariable(c_name, kCGcc, gcc_value, clang_value);
}


}  // namespace repobuild
