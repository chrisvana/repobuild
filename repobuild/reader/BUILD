[
 { "cc_library": {
     "name" : "buildfile",
     "cc_sources" : [ "buildfile.cc" ],
     "cc_headers" : [ "buildfile.h" ],
     "dependencies": [ "//common/base:macros",
                       "//common/log:log",
                       "//common/file:fileutil",
                       "//common/strings:strutil",
                       "//repobuild/env:resource",
                       "//repobuild/json:json"
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
                       "//repobuild/env:input",
                       "//repobuild/env:target",
                       "//repobuild/json:json",
                       "//repobuild/nodes:allnodes",
                       ":buildfile"
     ]
   }
 }
]