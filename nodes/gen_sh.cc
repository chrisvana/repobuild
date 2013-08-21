// Copyright 2013
// Author: Christopher Van Arsdale

#include <set>
#include <string>
#include <vector>
#include "common/log/log.h"
#include "common/strings/path.h"
#include "common/strings/strutil.h"
#include "repobuild/env/input.h"
#include "repobuild/nodes/gen_sh.h"
#include "repobuild/reader/buildfile.h"

using std::string;
using std::vector;
using std::set;

namespace repobuild {

void GenShNode::Parse(BuildFile* file, const BuildFileNode& input) {
  Node::Parse(file, input);
  if (!ParseStringField(input, "build_cmd", &build_cmd_)) {
    LOG(FATAL) << "Could not parse build_cmd.";
  }
  ParseStringField(input, "clean", &clean_cmd_);
  ParseRepeatedFiles(input, "input_files", &input_files_);
  ParseRepeatedString(input, "outs", &outputs_);
}

void GenShNode::Set(const std::string& build_cmd,
                    const std::string& clean_cmd,
                    const std::vector<std::string>& input_files,
                    const std::vector<std::string>& outputs) {
  build_cmd_ = build_cmd;
  clean_cmd_ = clean_cmd;
  input_files_ = input_files;
  outputs_ = outputs;
}

void GenShNode::WriteMakefile(const vector<const Node*>& all_deps,
                              string* out) const {
  // Target
  string touchfile = Touchfile();
  out->append(touchfile);
  out->append(": ");

  // Input files
  {
    set<string> input_files;
    CollectDependencies(all_deps, &input_files);
    input_files.erase(touchfile);  // all but our own file.
    out->append(strings::Join(input_files, " "));
    out->append(" ");
    out->append(strings::Join(input_files_, " "));
    out->append("\n");
  }

  // Build command.
  if (!build_cmd_.empty()) {
    out->append("\t@echo Script: ");
    out->append(target().full_path());
    out->append("\n\t@");

    string touch_cmd = "mkdir -p " +
        strings::JoinPath(input().object_dir(), target().dir()) +
        "; touch " + touchfile;

    string prefix;
    {  // compute prefix.
      set<string> compile_flags;
      CollectCompileFlags(true, all_deps, &compile_flags);
      prefix = "DEP_CXXFLAGS= " + strings::Join(compile_flags, " ");
      compile_flags.clear();
      CollectCompileFlags(false, all_deps, &compile_flags);
      prefix += " DEP_CFLAGS= " + strings::Join(compile_flags, " ");
    }

    out->append(WriteCommand(prefix, build_cmd_, touch_cmd));
    out->append("\n");
  }
  out->append("\n");

  {  // user target
    set<string> output_targets;
    output_targets.insert(touchfile);
    out->append(WriteBaseUserTarget(output_targets));
  }

  for (const string& output : outputs_) {
    out->append(strings::JoinPath(GenDir(), output));
    out->append(": ");
    out->append(touchfile);
    out->append("\n\n");
  }

  out->append(".PHONY: ");
  out->append("\n\n");
}

void GenShNode::WriteMakeClean(std::string* out) const {
  if (clean_cmd_.empty()) {
    return;
  }

  out->append("\t");
  out->append(WriteCommand("", clean_cmd_, ""));
  out->append("\n");
}

namespace {
void AddEnvVar(const string& var, string* out) {
  out->append(" ");
  out->append(var);
  out->append("=\"$(");
  out->append(var);
  out->append(")\"");
}
}

string GenShNode::WriteCommand(const string& prefix,
                               const string& cmd,
                               const string& admin_cmd) const {
  string out;
  out.append("(mkdir -p ");
  out.append(GenDir());
  if (cd_ && !target().dir().empty()) {
    out.append("; cd ");
    out.append(target().dir());
  }

  // Environment.
  out.append("; GEN_DIR=\"");
  out.append(cd_ ? RelativeGenDir() : GenDir());
  out.append("\"");
  out.append(" OBJ_DIR=\"");
  out.append(cd_ ? RelativeObjectDir() : ObjectDir());
  out.append("\"");
  AddEnvVar("CXX_GCC", &out);
  AddEnvVar("CC_GCC", &out);
  AddEnvVar("CC", &out);
  AddEnvVar("CXX", &out);
  AddEnvVar("CXXFLAGS", &out);
  AddEnvVar("BASIC_CXXFLAGS", &out);
  AddEnvVar("CFLAGS", &out);
  AddEnvVar("BASIC_CFLAGS", &out);
  AddEnvVar("LDFLAGS", &out);
  AddEnvVar("MAKE", &out);
  out.append(" ");
  out.append(prefix);

  // TODO: Pass up header_compile_args from dependencies as DEP_FLAGS

  // Execute command
  out.append(" eval '");
  out.append(MakefileEscape(cmd));

  out.append("')");
  if (!admin_cmd.empty()) {
    out.append(" && (");
    out.append(admin_cmd);
    out.append(")");
  }
  return out;
}

void GenShNode::DependencyFiles(vector<string>* files) const {
  files->push_back(Touchfile());
}

string GenShNode::Touchfile() const {
  return strings::JoinPath(
      strings::JoinPath(input().object_dir(), target().dir()),
      "." + target().local_path() + ".dummy");
}


}  // namespace repobuild
