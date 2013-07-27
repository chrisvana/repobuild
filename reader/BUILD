[
 { "cc_library": {
     "name" : "buildfile",
     "cc_sources" : [ "buildfile.cc" ],
     "cc_headers" : [ "buildfile.h" ],
     "dependencies": [ "//common/log:log",
                       "//json:json"
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
                       "//env:input",
                       "//json:json",
                       ":buildfile"
     ]
   }
 }
]