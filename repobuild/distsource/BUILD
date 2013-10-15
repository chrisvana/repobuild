[
{ "cc_library": {
    "name": "git_tree",
    "cc_headers": [ "git_tree.h" ],
    "cc_sources": [ "git_tree.cc" ],
    "dependencies":  [ "//common/base:base",
                       "//common/util:shell",
                       "//common/util:stl",
                       "//common/strings:strutil",
                       "//repobuild/third_party/libgit2:libgit2",
                       "//repobuild/nodes:makefile"
    ]
} },

{ "cc_library": {
    "name": "dist_source",
    "cc_headers": [ "dist_source.h" ],
    "dependencies": [ "//repobuild/nodes:makefile" ]
} },

{ "cc_library": {
    "name": "dist_source_impl",
    "cc_headers": [ "dist_source_impl.h" ],
    "cc_sources": [ "dist_source_impl.cc" ],
    "dependencies":  [ "//common/base:base",
                       "//common/file:fileutil",
                       ":dist_source",
                       ":git_tree"
    ]
} }
]
