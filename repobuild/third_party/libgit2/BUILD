[
{ "cmake": {
   "name": "libgit2_cmake",
   "cmake_args": [ "-DBUILD_SHARED_LIBS=OFF" ],
   "outs": [
     "build/libgit2.a"
   ]
} },
{ "cc_library": {
   "name": "libgit2",
   "strict_file_mode": false,
   "cc_objects": [ "$GEN_DIR/build/libgit2.a" ],
   "cc_headers": [ "include/git2.h",
                   "include/*/*.h",
                   "include/*/*/*.h" ],
   "include_dirs": [ "include" ],
   "dependencies": [ ":libgit2_cmake" ]
} }
]