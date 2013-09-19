// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_ENV_RESOURCE_H__
#define _REPOBUILD_ENV_RESOURCE_H__

#include <iosfwd>
#include <vector>
#include <set>
#include <string>

namespace repobuild {

class Resource {
 public:
  static Resource FromRootPath(const std::string& root_path);
  static Resource FromRaw(const std::string& raw_name);  // no dirname/basename
  static Resource FromLocalPath(const std::string& path,
                                const std::string& local);

  Resource() {}
  ~Resource() {}

  // accessors
  const std::string& path() const { return root_path_; }
  const std::string& basename() const { return basename_; }
  const std::string& dirname() const { return dirname_; }

  bool has_tag(const std::string& tag) const {
    return tags_.find(tag) != tags_.end();
  }
  const std::set<std::string>& tags() const { return tags_; }

  // mutators
  void CopyTags(const Resource& other) { tags_ = other.tags_; }
  std::set<std::string>* mutable_tags() { return &tags_; }
  void add_tag(const std::string& tag) { tags_.insert(tag); }

  // STL stuff (for set<>, etc) ----
  bool operator<(const Resource& other) const {
    return root_path_ < other.root_path_;
  }
  bool operator==(const Resource& other) const {
    return root_path_ == other.root_path_;
  }

 private:
  std::string root_path_, basename_, dirname_;
  std::set<std::string> tags_;
};

extern std::ostream& operator<<(std::ostream& o, const Resource& r);

class ResourceFileSet {
 public:
  ResourceFileSet() {}
  ~ResourceFileSet() {}

  // Files.
  const std::vector<Resource>& files() const { return files_; }
  void Add(const Resource& resource) {
    if (fileset_.insert(resource).second) {
      files_.push_back(resource);
    }
  }
  template <class T>
  void AddRange(const T& t) {
    for (const Resource& it : t) {
      Add(it);
    }
  }

  std::vector<Resource>::const_iterator begin() const { return files_.begin(); }
  std::vector<Resource>::const_iterator end() const { return files_.end(); }

 private:
  std::set<Resource> fileset_;
  std::vector<Resource> files_;
};

}  // namespace repobuild

#endif  // _REPOBUILD_ENV_RESOURCE_H__
