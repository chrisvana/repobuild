// Test objects
[

 { "proto_library": {
     "name" : "a_proto",
     "sources" : [ "a.proto" ]
   }
 },

 { "cc_library": {
     "name" : "foo_pred_a",
     "cc_sources" : [ "foo.cc" ],
     "cc_headers" : [ "foo.h" ],
     "dependencies" : [ ":a_proto" ]
   }
 },

 { "cc_library": {
     "name" : "bar_pred_a",
     "cc_sources" : [ "bar.cc" ]
   }
 }

]