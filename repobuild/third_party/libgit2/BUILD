[
{ "make": {
   "name": "libgit2_make",
   "make_file": "Makefile.embed",
   "make_target": "all",
   "postinstall": "mv libgit2.a $GEN_DIR/",
   "outs": [
     "libgit2.a"
   ]
} },
{ "cc_library": {
   "name": "libgit2",
   "strict_file_mode": false,
   "cc_objects": [ "$GEN_DIR/libgit2.a" ],
   "cc_headers": [ "include/git2.h",
                   "include/*/*.h",
                   "include/*/*/*.h" ],
   "include_dirs": [ "include" ],
   "dependencies": [ ":libgit2_make" ]
} }
]