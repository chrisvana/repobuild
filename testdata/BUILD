// Test objects
[
 { "cc_library": {
     "name" : "foo",
     "cc_sources" : [ "foo.cc" ],
     "cc_headers" : [ "foo.h" ],
     "dependencies": [ "//a:foo_pred_a", "//b:foo_pred_b" ]
   }
 },

 { "cc_library": {
     "name" : "bar",
     "cc_sources" : [ "bar.cc" ],
     "cc_headers" : [ "bar.h" ],
     "dependencies": [ "//b:bar_pred_b" ],
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