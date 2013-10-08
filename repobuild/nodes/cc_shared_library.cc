// Copyright 2013
// Author: Christopher Van Arsdale
//
// TODO(cvanarsdale): This overlaps a bunch with cc_binary.

#include <algorithm>
#include <set>
#include <string>
#include <vector>
#include <iterator>
#include "common/log/log.h"
#include "common/strings/path.h"
#include "repobuild/env/input.h"
#include "repobuild/env/resource.h"
#include "repobuild/nodes/cc_shared_library.h"
#include "repobuild/nodes/top_symlink.h"
#include "repobuild/reader/buildfile.h"

using std::string;
using std::vector;
using std::set;

namespace repobuild {
namespace {
// TODO(cvanarsdale): Consolidate with cc_library.cc:
const char kCxxGcc[] = "CXX_GCC";
const char kIsDarwin[] = "IS_DARWIN";
const char kIsDarwinAndClang[] = "IS_DARWIN_AND_CLANG";
}

void CCSharedLibraryNode::Parse(BuildFile* file, const BuildFileNode& input) {
  CCLibraryNode::Parse(file, input);

  // cc_sources
  vector<Resource> tmp_symbols;
  current_reader()->ParseSingleFile("exported_symbols_file", &tmp_symbols);
  if (tmp_symbols.size() > 1) {
    LOG(FATAL) << "exported_symbols_file must match exactly 1 file in "
               << target().full_path()
               << ". Found " << tmp_symbols.size() << " files.";
  } else if (tmp_symbols.size() == 1) {
    exported_symbols_ = tmp_symbols[0];
  }

  // Versioning.
  current_reader()->ParseStringField("release_version", &release_version_);
  current_reader()->ParseStringField("minor_version", &minor_version_);
  current_reader()->ParseStringField("major_version", &major_version_);
  if (!release_version_.empty() && minor_version_.empty()) {
    minor_version_ = "0";
  }
  if (major_version_.empty() && !minor_version_.empty()) {
    major_version_ = "0";
  }
}

void CCSharedLibraryNode::LocalWriteMake(Makefile* out) const {
  CCLibraryNode::LocalWriteMakeInternal(false, out);
  WriteLink(out);
  ResourceFileSet res;
  res.Add(OutLinkedObj());
  WriteBaseUserTarget(res, out);
}

void CCSharedLibraryNode::WriteLink(Makefile* out) const {
  ResourceFileSet objects;
  CCLibraryNode::ObjectFiles(CPP, &objects);

  set<string> flags;
  LinkFlags(CPP, &flags);

  // Link rule
  Resource file = OutLinkedObj();
  Makefile::Rule* rule = out->StartRule(file.path(),
                                        strings::JoinAll(objects.files(), " "));
  rule->WriteUserEcho("Linking", file.path());

  // HACK(cvanarsdale):
  // Sadly order matters to the (gcc) linker. It looks in later object
  // files to find unresolved symbols. We collect the dependencies
  // bottom up, so we push resources onto the front of the list so
  // unencumbered resources end up in the back of the list.
  vector<Resource> copy = objects.files();
  std::reverse(copy.begin(), copy.end());
  string obj_list = strings::JoinAll(copy, " ");

  // Ugly:
  // We let the makefile figure out the output shared object name.
  string shared_obj, link_path;
  if (release_version_ != "") {
    shared_obj = strings::StringPrintf(
        "$$(printf '%s %s %s %s %s' | $(SHARED_LIB_ARGS_R))",
        file.dirname().c_str(), target().local_path().c_str(),
        major_version_.c_str(), minor_version_.c_str(),
        release_version_.c_str());
    link_path = strings::StringPrintf(
        "$$(printf '%s %s %s %s' | $(SHARED_LIB_NAME_R))",
        target().local_path().c_str(),
        major_version_.c_str(), minor_version_.c_str(),
        release_version_.c_str());
  } else if (minor_version_ != "") {
    shared_obj = strings::StringPrintf(
        "$$(printf '%s %s %s %s' | $(SHARED_LIB_ARGS_MI))",
        file.dirname().c_str(), target().local_path().c_str(),
        major_version_.c_str(), minor_version_.c_str());
    link_path = strings::StringPrintf(
        "$$(printf '%s %s %s' | $(SHARED_LIB_NAME_MI))",
        target().local_path().c_str(),
        major_version_.c_str(), minor_version_.c_str());
  } else if (major_version_ != "") {
    shared_obj = strings::StringPrintf(
        "$$(printf '%s %s %s' | $(SHARED_LIB_ARGS_MA))",
        file.dirname().c_str(), target().local_path().c_str(),
        major_version_.c_str());
    link_path = strings::StringPrintf(
        "$$(printf '%s %s' | $(SHARED_LIB_NAME_MA))",
        target().local_path().c_str(), major_version_.c_str());
  } else {
    shared_obj = strings::StringPrintf(
        "$$(printf '%s %s' | $(SHARED_LIB_ARGS))",
        file.dirname().c_str(), target().local_path().c_str());
    link_path = strings::StringPrintf(
        "$$(printf '%s' | $(SHARED_LIB_NAME))",
        target().local_path().c_str());
  }

  string exported_symbols;
  if (!exported_symbols_.path().empty()) {
    exported_symbols = strings::StringPrintf(
        "$$($(EXPORTED_SYMBOLS) %s", exported_symbols_.path().c_str());
  }

  rule->WriteCommand("mkdir -p " + file.dirname());
  rule->WriteCommand(strings::JoinWith(
      " ",
      "$(LINK.cc)", obj_list, exported_symbols, shared_obj,
      strings::JoinAll(flags, " ")));
  rule->WriteCommand("ln -f -s " + link_path + " " + file.path());
  out->FinishRule(rule);
}

// static
void CCSharedLibraryNode::WriteMakeHead(const Input& input, Makefile* out) {
  // SHARED_LIB_NAME_R;
  // SHARED_LIB_NAME_MI;
  // SHARED_LIB_NAME_MA;
  // SHARED_LIB_NAME;
  // SHARED_LIB_ARGS_R;
  // SHARED_LIB_ARGS_MI;
  // SHARED_LIB_ARGS_MA;
  // SHARED_LIB_ARGS;
  out->append("# Some platform specific flag settings.\n");

  out->append(string(kIsDarwin) + " := $(shell echo $$("
              "uname | grep 'Darwin' | wc -l))\n");
  out->append(string(kIsDarwinAndClang) + " := $(shell echo $$((("
              "$(" + string(kIsDarwin) + ") == 1 && "
              "$(" + string(kCxxGcc) + ") == 0))))\n");

  out->append("ifeq ($(" + string(kIsDarwinAndClang) + "),1)\n");

  // Darwin and Clang.
  // SHARED_LIB_ARGS
  out->append("\tSHARED_LIB_ARGS_R:=awk '{print \"-dynamiclib -current_version"
              " \"$$4\" -compatibility_version \"$$5\" -o \"$$1\"/lib\"$$2\"."
              "\"$$3\".\"$$4\".\"$$5\".dylib\"}'\n");
  out->append("\tSHARED_LIB_ARGS_MI:=awk '{print \"-dynamiclib -current_version"
              " \"$$4\" -o \"$$1\"/lib\"$$2\".\"$$3\".\"$$4\".dylib\"}'\n");
  out->append("\tSHARED_LIB_ARGS_MA:=awk '{print \"-dynamiclib -o "
              "\"$$1\"/lib\"$$2\".\"$$3\".dylib\"}'\n");
  out->append("\tSHARED_LIB_ARGS:=awk '{print \"-dynamiclib -o "
              "\"$$1\"/lib\"$$2\".dylib\"}'\n");

  // SHARED_LIB_NAME
  out->append("\tSHARED_LIB_NAME_R:=awk '{print \"lib\"$$1\".\"$$2\".\"$$3\""
              ".\"$$4\".dylib\"}'\n");
  out->append("\tSHARED_LIB_NAME_MI:=awk '{print \"lib\"$$1\".\"$$2\".\"$$3\""
              ".dylib\"}'\n");
  out->append("\tSHARED_LIB_NAME_MA:=awk '{print \"lib\"$$1\".\"$$2\""
              ".dylib\"}'\n");
  out->append("\tSHARED_LIB_NAME:=awk '{print \"lib\"$$1\".dylib\"}'\n");

  out->append("else\n");

  // GCC
  // SHARED_LIB_ARGS
  out->append("\tSHARED_LIB_ARGS_R:=awk '{print \"-shared -Wl,-soname,lib\"$$2"
              "\".so.\"$$3\" -o \"$$1\"/lib\"$$2\".so.\"$$3\".\"$$4\".\"$$5"
              "\"}'\n");
  out->append("\tSHARED_LIB_ARGS_MA:=awk '{print \"-shared -Wl,-soname,lib\"$$2"
              "\".so.\"$$3\" -o \"$$1\"/lib\"$$2\".so.\"$$3\".\"$$4}'\n");
  out->append("\tSHARED_LIB_ARGS_MI:=awk '{print \"-shared -Wl,-soname,lib\"$$2"
              "\".so.\"$$3\" -o \"$$1\"/lib\"$$2\".so.\"$$3}'\n");
  out->append("\tSHARED_LIB_ARGS:=awk '{print \"-shared -Wl,-soname,lib\"$$2"
              "\".so -o \"$$1\"/lib\"$$2\".so\"}'\n");

  // SHARED_LIB_NAME
  out->append("\tSHARED_LIB_NAME_R:=awk '{print \"lib\"$$1\".so.\"$$2\".\"$$3"
              "\".\"$$4}'\n");
  out->append("\tSHARED_LIB_NAME_MI:=awk '{print \"lib\"$$1\".so.\"$$2\".\"$$3"
              "}'\n");
  out->append("\tSHARED_LIB_NAME_MA:=awk '{print \"lib\"$$1\".so.\"$$2}'\n");
  out->append("\tSHARED_LIB_NAME:=awk '{print \"lib\"$$1\".so\"}'\n");

  out->append("endif\n");
}

void CCSharedLibraryNode::ObjectFiles(LanguageType lang,
                                      ResourceFileSet* files) const {
  // Don't inherit anything.
  files->Add(OutLinkedObj());
}

Resource CCSharedLibraryNode::OutLinkedObj() const {
  string basename = "lib" + target().local_path() + "." +
      strings::JoinWith(".",
                        "so", major_version_, minor_version_, release_version_);
  return Resource::FromLocalPath(
      input().object_dir(),
      strings::JoinPath(target().dir(), basename));
}

}  // namespace repobuild
