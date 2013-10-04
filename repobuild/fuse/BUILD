[
 { "cc_binary": {
   "name": "fuse",
   "cc_sources": [ "fuse.cc" ],
   "dependencies": [ "//repobuild/third_party/libgit2:libgit2",
                     "//common/base:base",
                     "//common/strings:strutil"
                   ],
   "cc_compile_args": [ "-I/usr/local/include/osxfuse",
                        "-D_FILE_OFFSET_BITS=64",
                        "-DFUSE_USE_VERSION=26",
                        "-D_DARWIN_USE_64_BIT_INODE" ],
   "cc_linker_args": [ "-losxfuse" ]
 } }
]