// Copyright 2013
// Author: Christopher Van Arsdale

#include <fuse.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <map>
#include "common/base/init.h"
#include "common/base/flags.h"
#include "common/log/log.h"
#include "common/strings/strutil.h"

using std::set;
using std::string;
using std::vector;
using std::map;

DEFINE_string(fuse_args, "",
              "Arguments to pass to fuse, space separated.");

static set<string> dirs() {
  set<string> d;
  d.insert("/");
  d.insert("/dir1/");
  d.insert("/dir2/");
  d.insert("/dir2/subdir1/");
  d.insert("/dir2/subdir2/");
  return d;
}

static map<string, string> files() {
  map<string, string> d;
  d["/top_file"] = "contents!";
  d["/dir1/1_file"] = "contents!";
  d["/dir2/2_file"] = "contents!";
  d["/dir2/subdir1/s_1_file"] = "contents!";
  d["/dir2/subdir2/s_2_file"] = "contents!";
  d["/dir2/subdir2/s_2_otherfile"] = "contents!";
  return d;
}

template <typename T>
static bool HasValue(const T& value, const string& prefix) {
  return value.find(prefix) != value.end();
}

static int hello_getattr(const char *path, struct stat *stbuf) {
  memset(stbuf, 0, sizeof(struct stat));
  if (HasValue(dirs(), path) || HasValue(dirs(), string(path) + "/")) {
    stbuf->st_mode = S_IFDIR | 0755;
    stbuf->st_nlink = 3;
    return 0;
  }

  map<string, string> f = files();
  auto it = f.find(path);
  if (it != f.end()) {
    stbuf->st_mode = S_IFREG | 0444;
    stbuf->st_nlink = 1;
    stbuf->st_size = it->second.size();
    return 0;
  }
  return -ENOENT;
}

static int hello_open(const char *path, struct fuse_file_info *fi) {
  map<string, string> f = files();
  auto it = f.find(path);
  if (it == f.end()) {
     return -ENOENT;
  }
  if ((fi->flags & O_ACCMODE) != O_RDONLY) {
    // Only reading allowed.
    return -EACCES;
  }
  return 0;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi) {
  set<string> d = dirs();
  auto it = d.find(path);
  if (it == d.end()) {
    it = d.find(string(path) + "/");
  }
  if (it == d.end()) {
    return -ENOENT;
  }
  const string& dir = *it;

  filler(buf, ".", NULL, 0);           /* Current directory (.)  */
  filler(buf, "..", NULL, 0);          /* Parent directory (..)  */
  for (auto it : files()) {
    const string& file = it.first;
    if (strings::HasPrefix(file, dir)) {
      string trimmed = file.substr(dir.size());
      if (!trimmed.empty() && trimmed.find('/') == string::npos) {
        filler(buf, trimmed.c_str(), NULL, 0);
      }
    }
  }
  for (const string& subdir : dirs()) {
    if (strings::HasPrefix(subdir, dir)) {
      string trimmed = subdir.substr(dir.size(),
                                     subdir.size() - dir.size() - 1);
      if (!trimmed.empty() && trimmed.find('/') == string::npos) {
        filler(buf, trimmed.c_str(), NULL, 0);
      }
    }
  }

  return 0;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi) {
  map<string, string> f = files();
  auto it = f.find(path);
  if (it == f.end()) {
    return -ENOENT;
  }
  const string& data = it->second;

  if (offset >= data.size()) {
    return 0;
  }

  if (offset + size > data.size()) {
    // Trim the read to the file size.
    size = data.size() - offset;
  }

  memcpy(buf, data.data() + offset, size);
  return size;
}

static struct fuse_operations hello_filesystem_operations = {
    .getattr = hello_getattr, /* To provide size, permissions, etc. */
    .open    = hello_open,    /* To enforce read-only access.       */
    .read    = hello_read,    /* To provide file content.           */
    .readdir = hello_readdir, /* To provide directory listing.      */
};

int main(int argc, char** argv) {
  InitProgram(&argc, &argv);
  vector<char*> args;
  for (int i = 0; i < argc; ++i) {
    args.push_back(argv[i]);
  }
  vector<string> fuse_args = strings::SplitString(FLAGS_fuse_args, " ");
  for (string& str : fuse_args) {
    args.push_back(&str[0]);
  }

  std::cout << "Starting fuse mount." << std::endl;
  return fuse_main(args.size(), &args[0], &hello_filesystem_operations, NULL);
}
