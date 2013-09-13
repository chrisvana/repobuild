// Test objects
[

 { "proto_library": {
     "name" : "a_proto",
     "sources" : [ "a.proto" ],
     "generate_cc": true,
     "generate_java": true,
     "generate_go": true,
     "generate_python": true
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
 },

{  "py_library": {
     "name": "test",
     "py_sources": [ "test.py" ],
     "dependencies": [ ":a_proto" ]
} },

{  "java_library": {
     "name": "test_java",
     "java_sources": [ "ATest.java" ],
     "dependencies": [ ":a_proto" ]
} }
]