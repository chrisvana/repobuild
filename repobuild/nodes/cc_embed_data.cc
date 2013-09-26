// Copyright 2013
// Author: Christopher Van Arsdale

#include <cctype>
#include <set>
#include <string>
#include <vector>
#include <iterator>
#include "common/log/log.h"
#include "common/strings/path.h"
#include "common/strings/strutil.h"
#include "repobuild/env/input.h"
#include "repobuild/nodes/cc_embed_data.h"
#include "repobuild/nodes/cc_library.h"
#include "repobuild/reader/buildfile.h"

using std::string;
using std::vector;

namespace repobuild {
namespace {
string EmbedScript(const Input& input) {
  return strings::JoinPath(input.genfile_dir(), "cc_embed.sh");
}
string RemoveNonAlpha(const string& input) {
  string base = input;
  for (int i = 0; i < base.size(); ++i) {
    if (!isalnum(base[i])) {
      base[i] = '_';
    }
  }
  return base;
}
string VariableName(const Resource& source) {
  return "embed_" + RemoveNonAlpha(source.basename());
}
}  // anonymous namespace

class CCEmbedDataNodeRaw : public Node {
 public:
  CCEmbedDataNodeRaw(const TargetInfo& t,
                     const Input& i)
      : Node(t, i) {
  }
  virtual ~CCEmbedDataNodeRaw() {}
  virtual void ParseWithPath(BuildFile* file,
                             const BuildFileNode& input,
                             const string& file_path);
  virtual void LocalWriteMake(Makefile* out) const;
  virtual void LocalDependencyFiles(LanguageType lang,
                                    ResourceFileSet* files) const;

  void GetOutputs(ResourceFileSet* sources,
                  ResourceFileSet* headers) const;

 protected:
  string NamespaceStart() const {
    string out = "\"";
    for (const string& name : namespaces_) {
      out += "namespace " + name + " { ";
    }
    return out + "\"";
  }
  string NamespaceEnd() const {
    return "\"" + strings::Repeat("} ", namespaces_.size()) + "\"";
  }

  Resource header_file_, source_file_;
  vector<string> namespaces_;
  vector<Resource> sources_;
};

void CCEmbedDataNodeRaw::ParseWithPath(BuildFile* file,
                                       const BuildFileNode& input,
                                       const string& file_path) {
  Node::Parse(file, input);
  current_reader()->ParseRepeatedFiles("files", &sources_);
  current_reader()->ParseRepeatedString("namespace", &namespaces_);
  header_file_ = Resource::FromLocalPath(Node::input().genfile_dir(),
                                         file_path + ".h");
  source_file_ = Resource::FromLocalPath(Node::input().genfile_dir(),
                                         file_path + ".cc");
}

void CCEmbedDataNodeRaw::LocalWriteMake(Makefile* out) const {
  Makefile::Rule* rule = out->StartRule(
      header_file_.path(),
      strings::JoinWith(" ", EmbedScript(input()),
                        strings::JoinAll(sources_, " ")));
  rule->WriteUserEcho("Embed", target().make_path());
  rule->WriteCommand("mkdir -p " + header_file_.dirname());

  // for f in "input variable" "input2 variable2" ...; do
  //   echo $f;
  // done | .gen-files/cc_embed.sh
  //           out_header out_cpp out_if_guard "namespace_start" "namespace_end"
  vector<string> inputs;
  for (const Resource& source : sources_) {
    inputs.push_back("\"" +
                     strings::JoinWith(
                         " ",
                         source.path(),
                         VariableName(source)) +
                     "\"");
  }
  rule->WriteCommand("for f in " + strings::JoinAll(inputs, " ") + "; do"
                     "  echo $$f;"
                     "done | " +
                     strings::JoinWith(
                         " ",
                         EmbedScript(input()),
                         header_file_.path(),
                         source_file_.path(),
                         strings::UpperString(
                             RemoveNonAlpha(StripSpecialDirs(
                                 header_file_.path()))),
                         NamespaceStart(),
                         NamespaceEnd()));
  out->FinishRule(rule);

  // cc file depends on header file. Originally this was part of StartRule
  // above, but it tickled a bug in make that executed the script twice
  // (and overwrote the file with bad data).
  out->WriteRule(source_file_.path(), header_file_.path());

  ResourceFileSet files;
  files.Add(source_file_);
  files.Add(header_file_);
  WriteBaseUserTarget(files, out);
}

void CCEmbedDataNodeRaw::LocalDependencyFiles(LanguageType lang,
                                              ResourceFileSet* files) const {
  files->Add(header_file_);
  files->Add(source_file_);
}

void CCEmbedDataNodeRaw::GetOutputs(ResourceFileSet* sources,
                                    ResourceFileSet* headers) const {
  sources->Add(source_file_);
  headers->Add(header_file_);
}

CCEmbedDataNode::CCEmbedDataNode(const TargetInfo& target,
                                 const Input& input)
    : Node(target, input) {
}

void CCEmbedDataNode::Parse(BuildFile* file, const BuildFileNode& input) {
  // cc_library node
  CCLibraryNode* cc_library = NewSubNodeWithCurrentDeps<CCLibraryNode>(file);

  // embed node
  CCEmbedDataNodeRaw* embed_node = NewSubNode<CCEmbedDataNodeRaw>(file);
  embed_node->ParseWithPath(file, input, target().make_path());

  // Fill out cc_library
  cc_library->AddDependencyTarget(embed_node->target());
  ResourceFileSet sources, headers;
  embed_node->GetOutputs(&sources, &headers);
  cc_library->PreInitSources(sources, headers);
  cc_library->Parse(file, input);
}

// static
void CCEmbedDataNode::WriteMakeHead(const Input& input, Makefile* out) {
  const char kShellScript[] =
      "#!/bin/bash\n"
      "HEADER=\"$1\"\n"
      "CPP=\"$2\"\n"
      "GUARD=\"$3\"\n"
      "NAMESPACE_START=\"$4\"\n"
      "NAMESPACE_END=\"$5\"\n"
      // Header - start
      "echo \"#ifndef $GUARD\" > $HEADER\n"
      "echo \"#define $GUARD\" >> $HEADER\n"
      "echo \"#include <cstring>  // size_t\" >> $HEADER\n"
      "echo \"$NAMESPACE_START\" >> $HEADER\n"
      // Cpp - start
      "echo \"#include \\\"$(basename $HEADER)\\\"\" > $CPP\n"
      "echo \"$NAMESPACE_START\" >> $CPP\n"

      // Per-variable
      "while read SOURCE VARIABLE; do"
      //   Header
      "  echo \"// Auto generated from $SOURCE\" >> $HEADER\n"
      "  echo \"extern const char* \"$VARIABLE\"_data();\" >> $HEADER\n"
      "  echo \"extern size_t \"$VARIABLE\"_size();\" >> $HEADER\n"
      "  echo \"\" >> $HEADER\n"
      //   Cpp
      "  echo \"const char* \"$VARIABLE\"_data() {\" >> $CPP\n"
      "  printf \"  return \\\"\" >> $CPP\n"
      "  cat $SOURCE "
      "    | perl -pe 's|\\\\\\\\|\\\\\\\\\\\\\\\\|g' "
      "    | perl -pe 's|\\\\\"|\\\\\\\"|g' "
      "    | perl -pe 's|\\\\n|\\\\\\\\n|g' >> $CPP\n"
      "  echo \"\\\";\" >> $CPP\n"
      "  echo \"}\" >> $CPP\n"
      "  echo \"size_t \"$VARIABLE\"_size()\" { >> $CPP\n"
      "  printf \"  return \" >> $CPP\n"
      "  printf $(cat $SOURCE | wc -c) >> $CPP\n"
      "  echo \";\" >> $CPP\n"
      "  echo \"}\" >> $CPP\n"
      "done\n"

      // Header - end
      "echo \"$NAMESPACE_END\" >> $HEADER\n"
      "echo \"#endif  // $GUARD\" >> $HEADER\n"

      // Cpp - end
      "echo \"$NAMESPACE_END\" >> $CPP\n";
  out->GenerateExecFile("CCEmbed", EmbedScript(input),
                        Makefile::Escape(kShellScript));
}

}  // namespace repobuild
