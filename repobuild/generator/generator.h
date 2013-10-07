// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_GENERATOR_GENERATOR_H__
#define _REPOBUILD_GENERATOR_GENERATOR_H__

#include <string>

namespace repobuild {

class DistSource;
class Input;
class Parser;

class Generator {
 public:
  explicit Generator(DistSource* source);
  ~Generator();

  std::string GenerateMakefile(const Input& input);

 private:
  DistSource* source_;  // not owned
};

}  // namespace repobuild

#endif // _REPOBUILD_GENERATOR_GENERATOR_H__
