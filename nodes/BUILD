[
 { "cc_library": {
     "name" : "node",
     "cc_sources" : [ "node.cc" ],
     "cc_headers" : [ "node.h" ],
     "dependencies": [ "//common/log:log", "//reader:buildfile" ]
   }
 },

 { "cc_library": {
     "name" : "cc_library",
     "cc_sources" : [ "cc_library.cc" ],
     "cc_headers" : [ "cc_library.h" ],
     "dependencies": [ "//common/log:log",
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
     "name" : "allnodes",
     "cc_sources" : [ "allnodes.cc" ],
     "cc_headers" : [ "allnodes.h" ],
     "dependencies": [ ":node", ":cc_library", ":cc_binary" ]
   }
 }
]