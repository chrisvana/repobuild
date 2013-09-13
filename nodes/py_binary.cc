// Copyright 2013
// Author: Christopher Van Arsdale

#include <set>
#include <string>
#include <vector>
#include <iterator>
#include "common/log/log.h"
#include "common/strings/path.h"
#include "common/strings/strutil.h"
#include "repobuild/env/input.h"
#include "repobuild/nodes/py_binary.h"
#include "repobuild/reader/buildfile.h"

using std::string;
using std::vector;
using std::set;

namespace repobuild {
namespace {
string SetupFile(const Input& input) {
  return strings::JoinPath(input.genfile_dir(), "base_setup.py");
}
string GetPyModule(const string& source) {
  if (strings::HasSuffix(source, ".py")) {
    return strings::ReplaceAll(source.substr(0, source.size() - 3),
                               "/", ".");
  }
  if (strings::HasSuffix(source, ".pyc")) {
    return strings::ReplaceAll(source.substr(0, source.size() - 4),
                               "/", ".");
  }
  return "";
}
void WriteLocalLink(const Resource& original,
                    const Resource& final,
                    Makefile* out) {
  out->StartRule(final.path(), original.path());
  out->WriteCommand("pwd > /dev/null");  // hack to work around make issue?
  out->WriteCommand(strings::JoinWith(
      " ", "ln -f -s", original.path(), final.path()));
  out->FinishRule();
}
}

void PyBinaryNode::Parse(BuildFile* file, const BuildFileNode& input) {
  PyLibraryNode::Parse(file, input);
  current_reader()->ParseRepeatedString("py_build_args", &py_build_args_);
  current_reader()->ParseStringField("py_version", &py_version_);
  if (py_version_.empty()) {
    py_version_ = "1.0";
  }
  current_reader()->ParseStringField("py_default_module", &py_default_module_);
  if (py_default_module_.empty() && sources_.size() == 1) {
    py_default_module_ = GetPyModule(sources_[0].path());
  }
}

void PyBinaryNode::LocalWriteMake(Makefile* out) const {
  PyLibraryNode::LocalWriteMakeInternal(false, out);

  // Source files.
  ResourceFileSet deps;
  DependencyFiles(PYTHON, &deps);

  ResourceFileSet sources;
  ObjectFiles(PYTHON, &sources);
  vector<string> relative_sources;
  for (const Resource& r : sources.files()) {
    relative_sources.push_back(
        strings::JoinPath("$(ROOT_DIR)", r.path()));
  }

  // Output egg file
  Resource egg_touchfile = Touchfile(".egg");
  Resource egg_bin = EggBinary();
  out->StartRule(egg_touchfile.path(),
                 strings::Join(strings::JoinAll(deps.files(), " "), " ",
                               SetupFile(input())));
  out->WriteCommand("echo python build: " + egg_bin.path());
  out->WriteCommand("mkdir -p " + egg_touchfile.dirname());
  out->WriteCommand(
      "cd " + GenDir() + "; " +
      strings::JoinWith(
          " ",
          "PY_NAME=\"" + target().local_path() + "\"",
          "PY_VERSION=" + py_version_,
          "PY_SCRIPTS=\"" + strings::JoinAll(relative_sources, " ") + "\"",
          "python", strings::JoinPath("$(ROOT_DIR)", SetupFile(input())),
          "build",
          "--build-base=" + target().local_path() + ".build",
          "bdist_egg",
          "--dist-dir=" + strings::JoinPath("$(ROOT_DIR)", ObjectDir()),
          "--bdist-dir=" + target().local_path() + ".build",
          strings::JoinAll(py_build_args_, " ")));
  out->WriteCommand("touch " + egg_touchfile.path());
  out->FinishRule();

  // Link asdf-1.0-py2.7.egg to asdf.egg
  // TOOD(cvanarsdale): handle prefix case (e.g. asdf-1.0-bin1.egg, asdf.egg).
  out->StartRule(egg_bin.path(), egg_touchfile.path());
  out->WriteCommand(
      " cd " + ObjectDir() +
      "; ln -f -s $$(ls " + target().local_path() + "-*.egg) "
      + egg_bin.basename());
  out->FinishRule();

  // Script that runs .egg file.
  Resource bin = BinScript();
  out->StartRule(bin.path(), egg_bin.path());
  string module = py_default_module_.empty() ? "" : " -m " + py_default_module_;
  out->WriteCommand("echo 'python" + module +
                    " $$(pwd)/$$(dirname $$0)/" + egg_bin.basename() +
                    "' > " + bin.path() +
                    "; chmod 755 " + bin.path());

  // Symlink to that script in the root dir.
  WriteLocalLink(egg_bin, OutEgg(), out);

  // User target
  if (target().make_path() != OutBinary().path()) {
    ResourceFileSet user_deps;
    user_deps.Add(egg_bin);
    WriteBaseUserTarget(user_deps, out);
  }
  WriteLocalLink(bin, OutBinary(), out);
}

// static
void PyBinaryNode::WriteMakeHead(const Input& input, Makefile* out) {
  const char kPyScript[] =
      "import os\n"
      "from setuptools import setup, find_packages\n"
      "\n"
      "setup(\n"
      "    name = os.environ['PY_NAME'],\n"
      "    version = os.environ['PY_VERSION'],\n"
      "    packages = find_packages(),\n"
      "    scripts = os.environ['PY_SCRIPTS'].split(),\n"
      ")\n";
  out->append("define PythonSetup\n");
  out->append(kPyScript);
  out->append("\nendef\n");
  out->append("export PythonSetup\n");
  out->StartRule(SetupFile(input));
  out->WriteCommand("echo \"$$PythonSetup\" > " + SetupFile(input));
  out->FinishRule();
}

void PyBinaryNode::LocalWriteMakeClean(Makefile* out) const {
  out->WriteCommand("rm -f " + OutBinary().path());
  out->WriteCommand("rm -f " + OutEgg().path());
  out->WriteCommand("rm -rf " + strings::JoinPath(
      GenDir(),
      target().make_path() + ".egg-info"));
}

void PyBinaryNode::LocalDependencyFiles(LanguageType lang,
                                        ResourceFileSet* files) const {
  PyLibraryNode::LocalDependencyFiles(lang, files);
  LocalBinaries(lang, files);
}

void PyBinaryNode::LocalFinalOutputs(LanguageType lang,
                                     ResourceFileSet* outputs) const {
  outputs->Add(OutBinary());
  outputs->Add(OutEgg());
}

void PyBinaryNode::LocalBinaries(LanguageType lang,
                                 ResourceFileSet* outputs) const {
  outputs->Add(EggBinary());
}

Resource PyBinaryNode::OutBinary() const {
  return Resource::FromLocalPath(input().root_dir(),
                                 target().local_path());
}

Resource PyBinaryNode::OutEgg() const {
  return Resource::FromLocalPath(OutBinary().dirname(),
                                 OutBinary().basename() + ".egg");
}

Resource PyBinaryNode::EggBinary() const {
  return Resource::FromLocalPath(
      strings::JoinPath(input().object_dir(), target().dir()),
      target().local_path() + ".egg");
}

Resource PyBinaryNode::BinScript() const {
  return Resource::FromLocalPath(
      strings::JoinPath(input().object_dir(),
                        target().dir()),
      target().local_path());
}


}  // namespace repobuild
