[
 { "cc_library": {
     "name" : "makefile",
     "cc_sources" : [ "makefile.cc" ],
     "cc_headers" : [ "makefile.h" ],
     "dependencies": [ "//common/strings:strutil" ]
 } },

 { "cc_library": {
     "name": "util",
     "cc_sources": [ "util.cc" ],
     "cc_headers": [ "util.h" ],
     "dependencies": [ "//common/log:log",
                       "//common/strings:strutil",
                       "//repobuild/env:input",
                       "//repobuild/env:target"
     ]
 } },

 { "cc_library": {
     "name" : "node",
     "cc_sources" : [ "node.cc" ],
     "cc_headers" : [ "node.h" ],
     "dependencies": [ "//common/log:log",
                       "//common/file:fileutil",
                       "//common/strings:strutil",
                       "//common/util:stl",
                       "//repobuild/env:input",
                       "//repobuild/env:resource",
                       "//repobuild/env:target",
                       "//repobuild/reader:buildfile",
                       "//repobuild/third_party/json:json",
                       ":makefile",
                       ":util"
    ]
   }
 },

 { "cc_library": {
     "name" : "top_symlink",
     "cc_sources" : [ "top_symlink.cc" ],
     "cc_headers" : [ "top_symlink.h" ],
     "dependencies": [ "//repobuild/env:resource",
                       "//repobuild/env:input",
                       ":makefile",
                       ":node"
    ]
   }
 },

 { "cc_library": {
     "name" : "execute_test",
     "cc_sources" : [ "execute_test.cc" ],
     "cc_headers" : [ "execute_test.h" ],
     "dependencies": [ "//repobuild/env:resource",
                       "//repobuild/env:input",
                       ":gen_sh",
                       ":node"
    ]
   }
 },

 { "cc_library": {
     "name" : "confignode",
     "cc_sources" : [ "confignode.cc" ],
     "cc_headers" : [ "confignode.h" ],
     "dependencies": [ "//common/log:log",
                       "//common/strings:strutil",
                       ":node",
                       ":util"
      ]
   }
 },

 { "cc_library": {
     "name" : "cc_library",
     "cc_sources" : [ "cc_library.cc" ],
     "cc_headers" : [ "cc_library.h" ],
     "dependencies": [ "//common/log:log",
                       "//common/file:fileutil",
                       "//common/strings:strutil",
                       ":node"
      ]
   }
 },

 { "cc_library": {
     "name" : "cc_embed_data",
     "cc_sources" : [ "cc_embed_data.cc" ],
     "cc_headers" : [ "cc_embed_data.h" ],
     "dependencies": [ "//common/strings:strutil",
                       ":node"
      ]
   }
 },

 { "cc_library": {
     "name" : "cc_binary",
     "cc_sources" : [ "cc_binary.cc" ],
     "cc_headers" : [ "cc_binary.h" ],
     "dependencies": [ "//common/log:log",
                       "//common/strings:strutil",
                       ":node",
                       ":top_symlink"
      ]
   }
 },

 { "cc_library": {
     "name" : "cc_shared_library",
     "cc_sources" : [ "cc_shared_library.cc" ],
     "cc_headers" : [ "cc_shared_library.h" ],
     "dependencies": [ "//common/log:log",
                       "//common/strings:strutil",
                       ":node",
                       ":top_symlink",
                       ":util"
      ]
   }
 },
 { "cc_library": {
     "name" : "go_library",
     "cc_sources" : [ "go_library.cc" ],
     "cc_headers" : [ "go_library.h" ],
     "dependencies": [ "//common/strings:strutil",
                       ":node",
                       ":util"
     ]
   }
 },

 { "cc_library": {
     "name" : "go_binary",
     "cc_sources" : [ "go_binary.cc" ],
     "cc_headers" : [ "go_binary.h" ],
     "dependencies": [ "//common/log:log",
                       "//common/strings:strutil",
                       ":node"
      ]
   }
 },

 { "cc_library": {
     "name" : "go_test",
     "cc_sources" : [ "go_test.cc" ],
     "cc_headers" : [ "go_test.h" ],
     "dependencies": [ "//common/log:log",
                       "//common/strings:strutil",
                       ":node"
      ]
   }
 },

 { "cc_library": {
     "name" : "py_library",
     "cc_sources" : [ "py_library.cc" ],
     "cc_headers" : [ "py_library.h" ],
     "dependencies": [ "//common/log:log",
                       "//common/file:fileutil",
                       "//common/strings:strutil",
                       ":node",
                       ":util"
     ]
   }
 },

 { "cc_library": {
     "name" : "py_egg",
     "cc_sources" : [ "py_egg.cc" ],
     "cc_headers" : [ "py_egg.h" ],
     "dependencies": [ "//common/log:log",
                       "//common/strings:strutil",
                       ":py_library",
                       ":top_symlink"
      ]
   }
 },

 { "cc_library": {
     "name" : "py_binary",
     "cc_sources" : [ "py_binary.cc" ],
     "cc_headers" : [ "py_binary.h" ],
     "dependencies": [ "//common/log:log",
                       "//common/strings:strutil",
                       ":py_egg",
                       ":top_symlink"
      ]
   }
 },

 { "cc_library": {
     "name" : "java_library",
     "cc_sources" : [ "java_library.cc" ],
     "cc_headers" : [ "java_library.h" ],
     "dependencies": [ "//common/log:log",
                       "//common/strings:strutil",
                       ":node" ]
   }
 },

 { "cc_library": {
     "name" : "java_jar",
     "cc_sources" : [ "java_jar.cc" ],
     "cc_headers" : [ "java_jar.h" ],
     "dependencies": [ "//common/log:log",
                       "//common/strings:strutil",
                       ":java_library",
                       ":node",
                       ":top_symlink"
      ]
   }
 },

 { "cc_library": {
     "name" : "java_binary",
     "cc_sources" : [ "java_binary.cc" ],
     "cc_headers" : [ "java_binary.h" ],
     "dependencies": [ "//common/log:log",
                       "//common/strings:strutil",
                       ":java_jar",
                       ":top_symlink"
      ]
   }
 },

 { "cc_library": {
     "name" : "gen_sh",
     "cc_sources" : [ "gen_sh.cc" ],
     "cc_headers" : [ "gen_sh.h" ],
     "dependencies": [ "//common/base:flags",
                       "//common/log:log",
                       "//common/strings:strutil",
                       ":node"
      ]
   }
 },

 { "cc_library": {
     "name" : "proto_library",
     "cc_sources" : [ "proto_library.cc" ],
     "cc_headers" : [ "proto_library.h" ],
     "dependencies": [ "//common/log:log",
                       "//common/file:fileutil",
                       "//common/strings:strutil",
                       ":gen_sh"
      ]
   }
 },

 { "cc_library": {
     "name": "autoconf",
     "cc_sources" : [ "autoconf.cc" ],
     "cc_headers" : [ "autoconf.h" ],
     "dependencies": [ "//common/log:log",
                       "//common/file:fileutil",
                       "//common/strings:strutil",
                       ":gen_sh"
      ]
 } },

 { "cc_library": {
     "name": "make",
     "cc_sources" : [ "make.cc" ],
     "cc_headers" : [ "make.h" ],
     "dependencies": [ "//common/log:log",
                       "//common/file:fileutil",
                       "//common/strings:strutil",
                       ":gen_sh"
      ]
 } },

 { "cc_library": {
     "name": "cmake",
     "cc_sources" : [ "cmake.cc" ],
     "cc_headers" : [ "cmake.h" ],
     "dependencies": [ "//common/log:log",
                       "//common/file:fileutil",
                       "//common/strings:strutil",
                       ":gen_sh"
      ]
 } },

 { "cc_library": {
     "name" : "allnodes",
     "cc_sources" : [ "allnodes.cc" ],
     "cc_headers" : [ "allnodes.h" ],
     "dependencies": [ "//common/base:macros",
                       "//common/log:log",
                       "//common/util:stl",
                       ":autoconf",
                       ":cmake",
                       ":cc_binary",
                       ":cc_embed_data",
                       ":cc_library",
                       ":cc_shared_library",
                       ":confignode",
                       ":execute_test",
                       ":go_library",
                       ":go_binary",
                       ":go_test",
                       ":gen_sh",
                       ":java_binary",
                       ":java_library",
                       ":java_jar",
                       ":make",
                       ":node",
                       ":proto_library",
                       ":py_egg",
                       ":py_binary",
                       ":py_library"
     ]
   }
 }
]