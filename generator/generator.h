// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_GENERATOR_GENERATOR_H__
#define _REPOBUILD_GENERATOR_GENERATOR_H__

#include <memory>
#include <string>

namespace repobuild {

class Input;
class Parser;

class Generator {
 public:
  Generator();
  ~Generator();

  std::string GenerateMakefile(const Input& input);

 private:
  std::unique_ptr<Parser> parser_;
};

}  // namespace repobuild

#endif // _REPOBUILD_GENERATOR_GENERATOR_H__
