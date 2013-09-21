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
  virtual void Parse(BuildFile* file, const BuildFileNode& input);
  virtual void LocalWriteMake(Makefile* out) const;
  virtual void LocalDependencyFiles(LanguageType lang,
                                    ResourceFileSet* files) const;

  void GetOutputs(ResourceFileSet* sources,
                  ResourceFileSet* headers) const;

 protected:
  Resource HeaderFile(const Resource& original) const;
  Resource SourceFile(const Resource& original) const;

  std::string namespace_;
  std::vector<Resource> sources_;
};

void CCEmbedDataNodeRaw::Parse(BuildFile* file, const BuildFileNode& input) {
  Node::Parse(file, input);
  current_reader()->ParseRepeatedFiles("files", &sources_);
  current_reader()->ParseStringField("namespace", &namespace_);
}

void CCEmbedDataNodeRaw::LocalWriteMake(Makefile* out) const {
  ResourceFileSet files;
  for (const Resource& source : sources_) {
    Resource header_file = HeaderFile(source);
    Resource cpp_file = SourceFile(source);
    files.Add(header_file);
    files.Add(cpp_file);

    // Output egg file
    Makefile::Rule* rule = out->StartRule(
        strings::JoinWith(" ",
                          header_file,
                          cpp_file),
        strings::JoinWith(" ", EmbedScript(input()), source.path()));
    rule->WriteUserEcho("Embed", source.path());
    rule->WriteCommand("mkdir -p " + header_file.dirname());
    rule->WriteCommand(
        strings::JoinWith(" ",
                          EmbedScript(input()),
                          "$(ROOT_DIR)/" + source.path(),
                          header_file.path(),
                          cpp_file.path(),
                          VariableName(source),
                          strings::UpperString(
                              RemoveNonAlpha(StripSpecialDirs(
                                  header_file.path())))));
    out->FinishRule(rule);
  }

  WriteBaseUserTarget(files, out);
}

void CCEmbedDataNodeRaw::LocalDependencyFiles(LanguageType lang,
                          ResourceFileSet* files) const {
  for (const Resource& source : sources_) {
    files->Add(HeaderFile(source));
    files->Add(SourceFile(source));
  }
}

Resource CCEmbedDataNodeRaw::HeaderFile(const Resource& original) const {
  return Resource::FromLocalPath(
      input().genfile_dir(),
      StripSpecialDirs(original.path()) + ".h");
}

Resource CCEmbedDataNodeRaw::SourceFile(const Resource& original) const {
  return Resource::FromLocalPath(
      input().genfile_dir(),
      StripSpecialDirs(original.path()) + ".cc");
}

void CCEmbedDataNodeRaw::GetOutputs(ResourceFileSet* sources,
                                    ResourceFileSet* headers) const {
  for (const Resource& r : sources_) {
    sources->Add(SourceFile(r));
    headers->Add(HeaderFile(r));
  }
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
  embed_node->Parse(file, input);

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
      "SOURCE=\"$1\"\n"
      "HEADER=\"$2\"\n"
      "CPP=\"$3\"\n"
      "VARIABLE=\"$4\"\n"
      "GUARD=\"$5\"\n"
      // Header
      "echo \"// Auto generated from $SOURCE\" > $HEADER\n"
      "echo \"#ifndef $GUARD\" >> $HEADER\n"
      "echo \"#define $GUARD\" >> $HEADER\n"
      "echo \"#include <cstring>  // size_t\" >> $HEADER\n"
      "echo \"extern const char* \"$VARIABLE\"_data();\" >> $HEADER\n"
      "echo \"extern size_t \"$VARIABLE\"_size();\" >> $HEADER\n"
      "echo \"#endif  // $GUARD\" >> $HEADER\n"
      // Source
      "echo \"// Auto generated from $SOURCE\" > $CPP\n"
      "echo \"#include \\\"$(basename $HEADER)\\\"\" >> $CPP\n"
      "echo \"const char* \"$VARIABLE\"_data() {\" >> $CPP\n"
      "printf \"  return \\\"\" >> $CPP\n"
      // Rewriting of the source file so it can go in a c string.
      "cat $SOURCE "
      "| sed 's|\\\\\\\\|\\\\\\\\\\\\\\\\|g' "
      "| sed 's|\\\\\"|\\\\\\\"|g' "
      "| perl -pe 's|\\\\n|\\\\\\\\n|g' >> $CPP\n"
      "echo \"\\\";\" >> $CPP\n"
      "echo \"}\" >> $CPP\n"
      "echo \"size_t \"$VARIABLE\"_size()\" { >> $CPP\n"
      "printf \"  return \" >> $CPP\n"
      "printf $(cat $SOURCE | wc -c) >> $CPP\n"
      "echo \";\" >> $CPP\n"
      "echo \"}\" >> $CPP\n";
  out->GenerateExecFile("CCEmbed", EmbedScript(input),
                        Makefile::Escape(kShellScript));
}

}  // namespace repobuild
