// Test objects
[
 { "config": {
    "component": "testdata"
 } },

 { "cc_library": {
     "name" : "foo",
     "cc_sources" : [ "foo.cc" ],
     "cc_headers" : [ "foo.h" ],
     "dependencies": [ "//testdata/a:foo_pred_a",
                       "//testdata/b:foo_pred_b",
                       "//testdata/c:foo_pred_c",
                       "//testdata/d:foo_pred_d" 
     ]
   }
 },

 { "cc_library": {
     "name" : "bar",
     "cc_sources" : [ "ba*.cc" ],
     "cc_headers" : [ "bar.h" ],
     "dependencies": [ "//testdata/b:bar_pred_b" ],
     "cc_compile_args": [ "-O3" ]
   }
 },

 { "cc_binary": {
     "name" : "main",
     "cc_sources" : [ "main.cc" ],
     "dependencies": [ ":foo", ":bar" ],
     "cc_linker_args": [ "-lm" ]
   }
 }
]