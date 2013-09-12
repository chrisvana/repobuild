// Copyright 2013
// Author: Christopher Van Arsdale

#ifndef _REPOBUILD_ENV_RESOURCE_H__
#define _REPOBUILD_ENV_RESOURCE_H__

#include <iosfwd>
#include <list>
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
  // Type specification
  enum ResourceFileType {
    SOURCE = 0x1,
    CC_OBJ = 0x2,
    JAVA_OBJ = 0x4,
    ALL = 0x7
  };

  ResourceFileSet() : type_(ALL) {}
  ~ResourceFileSet() {}

  // Files.
  const std::list<Resource>& files() const { return files_; }
  void Add(const Resource& resource) {
    // HACK(cvanarsdale):
    // Sadly order matters to the (gcc) linker. It looks in later object
    // files to find unresolved symbols. We collect the dependencies
    // bottom up, so we push resources onto the front of the list so
    // unencumbered resources end up in the back of the list.
    if (fileset_.insert(resource).second) {
      files_.push_front(resource);
    }
  }

  // Types.
  ResourceFileType type() const { return type_; }
  void set_type(ResourceFileType type) { type_ = type; }

 private:
  std::set<Resource> fileset_;
  std::list<Resource> files_;
  ResourceFileType type_;
};

}  // namespace repobuild

#endif  // _REPOBUILD_ENV_RESOURCE_H__
