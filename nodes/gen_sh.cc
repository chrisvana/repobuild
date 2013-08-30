// Copyright 2013
// Author: Christopher Van Arsdale

#include <map>
#include <set>
#include <string>
#include <vector>
#include "common/base/flags.h"
#include "common/log/log.h"
#include "common/strings/path.h"
#include "common/strings/strutil.h"
#include "repobuild/env/input.h"
#include "repobuild/nodes/gen_sh.h"
#include "repobuild/reader/buildfile.h"

DEFINE_bool(silent_gensh, true,
            "If true, we only print out gen_sh stderr/stdout on "
            "script failures.");

using std::map;
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

void GenShNode::Set(const string& build_cmd,
                    const string& clean_cmd,
                    const vector<Resource>& input_files,
                    const vector<string>& outputs) {
  build_cmd_ = build_cmd;
  clean_cmd_ = clean_cmd;
  input_files_ = input_files;
  outputs_ = outputs;
}

void GenShNode::WriteMakefile(const vector<const Node*>& all_deps,
                              Makefile* out) const {
  Resource touchfile = Touchfile();

  // Inputs
  set<Resource> input_files;
  CollectDependencies(all_deps, &input_files);
  input_files.erase(touchfile);  // all but our own file.

  // Make target
  out->StartRule(touchfile.path(), strings::JoinWith(
      " ",
      strings::JoinAll(input_files, " "),
      strings::JoinAll(input_files_, " ")));

  // Build command.
  if (!build_cmd_.empty()) {
    out->WriteCommand("echo Script: " + target().full_path());

    string touch_cmd = "mkdir -p " +
        strings::JoinPath(input().object_dir(), target().dir()) +
        "; touch " + touchfile.path();

    string prefix;
    {  // compute prefix.
      set<string> compile_flags;
      CollectCompileFlags(true, all_deps, &compile_flags);
      prefix = "DEP_CXXFLAGS=\"" + strings::JoinAll(compile_flags, " ") + "\"";
      compile_flags.clear();
      CollectCompileFlags(false, all_deps, &compile_flags);
      prefix += " DEP_CFLAGS=\"" + strings::JoinAll(compile_flags, " ") + "\"";
    }

    map<string, string> env_vars;
    CollectEnvVariables(all_deps, &env_vars);
    out->WriteCommand(WriteCommand(env_vars, prefix, build_cmd_, touch_cmd));
  }
  out->FinishRule();

  {  // user target
    set<Resource> output_targets;
    output_targets.insert(touchfile);
    WriteBaseUserTarget(output_targets, out);
  }

  for (const string& output : outputs_) {
    out->WriteRule(Resource::FromLocalPath(GenDir(), output).path(),
                   touchfile.path());
  }
}

void GenShNode::WriteMakeClean(const vector<const Node*>& all_deps,
                               Makefile* out) const {
  if (clean_cmd_.empty()) {
    return;
  }

  map<string, string> env_vars;
  CollectEnvVariables(all_deps, &env_vars);
  out->WriteCommandBestEffort(WriteCommand(env_vars, "", clean_cmd_, ""));
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

string GenShNode::WriteCommand(const map<string, string>& env_vars,
                               const string& prefix,
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
  out.append(" SRC_DIR=\"");
  out.append(cd_ ? RelativeSourceDir() : SourceDir());
  out.append(" ROOT_DIR=\"");
  out.append(cd_ ? RelativeRootDir() : "./");

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
  for (const auto& it : env_vars) {
    out.append(" ");
    out.append(it.first);
    out.append("=\"");
    out.append(it.second);
    out.append("\"");
  }
  out.append(" ");
  out.append(prefix);

  // TODO: Pass up header_compile_args from dependencies as DEP_FLAGS

  // Execute command
  out.append(" eval '(");
  out.append(MakefileEscape(cmd));
  out.append(")'");

  // Logfile, if any
  if (FLAGS_silent_gensh) {
    string logfile = strings::JoinPath(cd_ ? RelativeGenDir() : GenDir(),
                                       ".logfile");
    out.append(" > " + logfile + " 2>&1 || (cat " + logfile + "; exit 1)");
  }

  out.append(" )");  // matches top "(".

  // Admin command, if any.
  if (!admin_cmd.empty()) {
    out.append(" && (");
    out.append(admin_cmd);
    out.append(")");
  }
  return out;
}

void GenShNode::DependencyFiles(vector<Resource>* files) const {
  files->push_back(Touchfile());
}

Resource GenShNode::Touchfile() const {
  return Resource::FromLocalPath(
      strings::JoinPath(input().object_dir(), target().dir()),
      "." + target().local_path() + ".dummy");
}


}  // namespace repobuild
