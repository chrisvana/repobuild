// Test objects
[
 { "gen_sh": {
     "name": "foo_gen",
     "build_cmd": "GEN_DIR=$GEN_DIR ./foo_gen.sh",
     "input_files": [ "foo_tmp.cc", "foo_tmp.h", "foo_gen.sh" ],
     "outs": [ "foo.cc", "foo.h" ]
 } },

 { "cc_library": {
     "name" : "foo_pred_d",
     "cc_headers" : [ "$GEN_DIR/foo.h" ],
     "cc_sources" : [ "$GEN_DIR/foo.cc" ],
     "dependencies": [ ":foo_gen" ],
     "strict_file_mode": false
   }
 }
]