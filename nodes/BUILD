[
 { "cc_library": {
     "name" : "node",
     "cc_sources" : [ "node.cc" ],
     "cc_headers" : [ "node.h" ],
     "dependencies": [ "//common/log:log",
                       "//common/file:fileutil",
                       "//common/strings:strutil",
                       "//reader:buildfile" ]
   }
 },

 { "cc_library": {
     "name" : "confignode",
     "cc_sources" : [ "confignode.cc" ],
     "cc_headers" : [ "confignode.h" ],
     "dependencies": [ "//common/log:log",
                       "//common/strings:strutil",
                       ":node"
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
     "name" : "cc_binary",
     "cc_sources" : [ "cc_binary.cc" ],
     "cc_headers" : [ "cc_binary.h" ],
     "dependencies": [ "//common/log:log",
                       "//common/strings:strutil",
                       ":node"
      ]
   }
 },


 { "cc_library": {
     "name" : "go_library",
     "cc_sources" : [ "go_library.cc" ],
     "cc_headers" : [ "go_library.h" ],
     "dependencies": [ ":node" ]
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
     "name" : "gen_sh",
     "cc_sources" : [ "gen_sh.cc" ],
     "cc_headers" : [ "gen_sh.h" ],
     "dependencies": [ "//common/log:log",
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
     "name" : "allnodes",
     "cc_sources" : [ "allnodes.cc" ],
     "cc_headers" : [ "allnodes.h" ],
     "dependencies": [ ":node",
                       ":cc_library",
                       ":cc_binary",
                       ":confignode",
                       ":go_library",
                       ":go_binary",
                       ":gen_sh",
                       ":proto_library"
     ]
   }
 }
]