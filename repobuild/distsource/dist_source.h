// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_DISTSOURCE_DIST_SOURCE_H__
#define _REPOBUILD_DISTSOURCE_DIST_SOURCE_H__

namespace repobuild {

class DistSource {
 public:
  DistSource() {}
  virtual ~DistSource() {}
  virtual void InitializeForFile(const std::string& file) = 0;
};

}  //  namespace repobuild

#endif  // _REPOBUILD_DISTSOURCE_DIST_SOURCE_H__
