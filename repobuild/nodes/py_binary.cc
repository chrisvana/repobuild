// Copyright 2013
// Author: Christopher Van Arsdale
//
// TODO(cvanarsdale): This overalaps a lot with other *_binary.cc files.

#include <set>
#include <string>
#include <vector>
#include <iterator>
#include "common/log/log.h"
#include "common/strings/path.h"
#include "common/strings/strutil.h"
#include "repobuild/env/input.h"
#include "repobuild/nodes/py_binary.h"
#include "repobuild/nodes/top_symlink.h"
#include "repobuild/reader/buildfile.h"

using std::string;
using std::vector;
using std::set;

namespace repobuild {
namespace {
string SetupFile(const Input& input) {
  return strings::JoinPath(input.pkgfile_dir(), "base_setup.py");
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
}  // anonymous namespace

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
  ResourceFileSet binaries;
  LocalBinaries(NO_LANG, &binaries);
  AddSubNode(new TopSymlinkNode(
      target().GetParallelTarget(file->NextName(target().local_path())),
      Node::input(),
      binaries));
}

void PyBinaryNode::LocalWriteMake(Makefile* out) const {
  PyLibraryNode::LocalWriteMakeInternal(false, out);
  {  // py_binary
    ResourceFileSet deps;
    vector<string> modules;
    GetSources(&deps, &modules);
    WriteEggFile(deps, modules, out);
  }
  WriteBaseUserTarget(out);
}

void PyBinaryNode::WriteEggFile(const ResourceFileSet& deps,
                                const vector<string>& modules,
                                Makefile* out) const {
  // Output egg file
  Resource egg_touchfile = Touchfile(".egg");
  Resource egg_bin = EggBinary();
  Makefile::Rule* rule =
      out->StartRule(egg_touchfile.path(),
                     strings::Join(strings::JoinAll(deps.files(), " "), " ",
                                   SetupFile(input())));
  rule->WriteUserEcho("Python build", egg_bin.path());
  rule->WriteCommand("mkdir -p " + egg_touchfile.dirname());
  rule->WriteCommand(
      "cd " + input().pkgfile_dir() + "; " +
      strings::JoinWith(
          " ",
          "PY_NAME=\"" + target().local_path() + "\"",
          "PY_VERSION=" + py_version_,
          "PY_MODULES=\"" + strings::JoinAll(modules, " ") + "\"",
          "python", strings::JoinPath("$(ROOT_DIR)", SetupFile(input())),
          "build",
          "--build-base=" + target().local_path() + ".build",
          "bdist_egg",
          "--dist-dir=" + strings::JoinPath("$(ROOT_DIR)", ObjectDir()),
          "--bdist-dir=" + target().local_path() + ".build",
          strings::JoinAll(py_build_args_, " ")));
  rule->WriteCommand("touch " + egg_touchfile.path());
  out->FinishRule(rule);

  // Link asdf-1.0-py2.7.egg to asdf.egg
  // TOOD(cvanarsdale): handle prefix case (e.g. asdf-1.0-bin1.egg, asdf.egg).
  rule = out->StartRule(egg_bin.path(), egg_touchfile.path());
  rule->WriteCommand(
      " cd " + ObjectDir() +
      "; ln -f -s $$(ls " + target().local_path() + "-*.egg) "
      + egg_bin.basename());
  out->FinishRule(rule);

  // Script that runs .egg file.
  Resource bin = BinScript();
  rule = out->StartRule(bin.path(), egg_bin.path());
  string module = py_default_module_.empty() ? "" : "-m " + py_default_module_;
  rule->WriteCommand("echo 'PYTHONPATH=$$(pwd)/$$(dirname $$0)/" +
                     egg_bin.basename() +":$$PYTHONPATH python " + module +
                     "' > " + bin.path() +
                     "; chmod 755 " + bin.path());
  out->FinishRule(rule);
}

void PyBinaryNode::GetSources(ResourceFileSet* deps,
                              vector<string>* modules) const {
  // Source files.
  PyLibraryNode::LocalDependencyFiles(PYTHON, deps);
  InputDependencyFiles(PYTHON, deps);

  ResourceFileSet sources;
  ObjectFiles(PYTHON, &sources);
  for (const Resource& r : sources.files()) {
    string path = StripSpecialDirs(r.path());
    if (strings::HasSuffix(path, ".py")) {
      path = path.substr(0, path.size() - 3);
    } else if (strings::HasSuffix(path, ".pyc")) {
      path = path.substr(0, path.size() - 4);
    }
    modules->push_back(strings::ReplaceAll(path, "/", "."));
  }

  // NB: Python setuptils is .... not the greatest fit in the world. It likes to
  // drop __init__.py files in directories that don't contain any actual
  // source files. This is either a bug or a quirk in how the developers
  // expected the packages to get installed. Either way, we need the __init__.py
  // files or it breaks later when the .egg file is run.
  set<string> init_files;
  for (const string& module : *modules) {
    vector<StringPiece> pieces = strings::Split(module, ".");
    while (!pieces.empty()) {
      pieces.resize(pieces.size() - 1);
      if (init_files.insert(strings::JoinWith(".",
                                              strings::JoinAll(pieces, "."),
                                              "__init__")).second) {
        break;
      }
    }
  }
  for (const string& init : init_files) {
    modules->push_back(init);
  }
}

// static
void PyBinaryNode::WriteMakeHead(const Input& input, Makefile* out) {
  const char kPyScript[] =
      "import os\n"
      "from setuptools import setup\n"
      "\n"
      "setup(\n"
      "    name = os.environ['PY_NAME'],\n"
      "    version = os.environ['PY_VERSION'],\n"
      "    py_modules = os.environ['PY_MODULES'].split(),\n"
      ")\n";
  out->GenerateExecFile("PythonSetup", SetupFile(input), kPyScript);
}

void PyBinaryNode::LocalBinaries(LanguageType lang,
                                 ResourceFileSet* outputs) const {
  outputs->Add(EggBinary());
  outputs->Add(BinScript());
}

Resource PyBinaryNode::EggBinary() const {
  return Resource::FromLocalPath(input().object_dir(),
                                 target().make_path() + ".egg");
}

Resource PyBinaryNode::BinScript() const {
  return Resource::FromLocalPath(input().object_dir(), target().make_path());
}


}  // namespace repobuild
