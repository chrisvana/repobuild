[
 { "cc_library": {
     "name" : "nodes",
     "cc_sources" : [ "nodes.cc" ],
     "cc_headers" : [ "nodes.h" ],
     "dependencies": [ "//common/log:log", "//reader:buildfile" ],
   }
 },

 { "cc_library": {
     "name" : "cc_library",
     "cc_sources" : [ "cc_library.cc" ],
     "cc_headers" : [ "cc_library.h" ],
     "dependencies": [ "//common/log:log", ":nodes" ],
   }
 },

 { "cc_library": {
     "name" : "cc_binary",
     "cc_sources" : [ "cc_binary.cc" ],
     "cc_headers" : [ "cc_binary.h" ],
     "dependencies": [ "//common/log:log", ":cc_library" ],
   }
 },

 { "cc_library": {
     "name" : "allnodes",
     "cc_sources" : [ "allnodes.cc" ],
     "cc_headers" : [ "allnodes.h" ],
     "dependencies": [ ":nodes", ":cc_library", ":cc_binary" ]
   }
 }
]