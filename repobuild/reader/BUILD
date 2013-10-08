[
 { "cc_library": {
     "name" : "buildfile",
     "cc_sources" : [ "buildfile.cc" ],
     "cc_headers" : [ "buildfile.h" ],
     "dependencies": [ "//common/base:macros",
                       "//common/log:log",
                       "//common/file:fileutil",
                       "//common/strings:strutil",
                       "//common/util:stl",
                       "//repobuild/distsource:dist_source",
                       "//repobuild/env:resource",
                       "//repobuild/env:target",
                       "//repobuild/third_party/json:json"
     ]
   }
 },

 { "cc_library": {
     "name" : "parser",
     "cc_sources" : [ "parser.cc" ],
     "cc_headers" : [ "parser.h" ],
     "dependencies": [ "//common/log:log",
                       "//common/file:fileutil",
                       "//common/strings:strutil",
                       "//common/util:stl",
                       "//repobuild/distsource:dist_source",
                       "//repobuild/env:input",
                       "//repobuild/env:target",
                       "//repobuild/nodes:allnodes",
                       "//repobuild/third_party/json:json",
                       ":buildfile"
     ]
   }
 }
]