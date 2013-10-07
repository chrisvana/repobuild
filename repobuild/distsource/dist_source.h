// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_DISTSOURCE_DIST_SOURCE_H__
#define _REPOBUILD_DISTSOURCE_DIST_SOURCE_H__

#include <string>
#include <vector>
#include "repobuild/nodes/makefile.h"

namespace repobuild {

class DistSource {
 public:
  DistSource() {}
  virtual ~DistSource() {}
  virtual void InitializeForFile(const std::string& glob,
                                 std::vector<std::string>* files) = 0;
  virtual void WriteMakeFile(Makefile* out) = 0;
  virtual void WriteMakeClean(Makefile::Rule* out) = 0;
};

}  //  namespace repobuild

#endif  // _REPOBUILD_DISTSOURCE_DIST_SOURCE_H__
