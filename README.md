repobuild
==========

Build tool for my projects.<br/>
<br/>
Mostly functional, but a work in progress (still in "beta" until [this](https://github.com/chrisvana/repobuild/wiki/Distributed-Source) is implemented).<br/>

--
_Summary_

Repobuild is a declarative style build system, similar to Google's BUILD file system of old (gconfig + make).<br/>

Further Reading:
* [Motivation](https://github.com/chrisvana/repobuild/wiki/Motivation) (my rants)
* [Similar build systems](https://github.com/chrisvana/repobuild/wiki/Similar-Build-Systems) (others like repobuild)

Current Languages:
- C++
- Python
- Java
- Go

--
_Building the tool_
```
# Build it:
$ git clone https://github.com/chrisvana/repobuild.git
$ cd repobuild
$ git submodule init
$ git submodule update
$ make -j8 repobuild

# Install somewhere in your path
$ # Example: sudo cp bin/repobuild  #/... somewhere in your path, like /usr/bin
$ sudo cp bin/repobuild /usr/bin/repobuild

# Usage:
$ repobuild --helpshort
$ ...

# Try it out on the testdata:
$ repobuild "testdata:go_main"
$ make
$ ./go_main

```

What should you do now?<br/>
- Look in testdata/BUILD, or repobuild/BUILD for some examples.<br/>
- Look at additional libraries are in "third_party".
- Start playing with your own project.
- You can add submodules/forks to "third_party" if you like.
- Forum: https://groups.google.com/forum/#!forum/repobuild

--
Why is this built using C++?<br/>
Actually this started in Go, and there is no good reason for C++. The first open source code I needed to use it on was in C++, and being able to build the tool using the tool sounded useful.

--
_TODOs:_<br/>
<br/>
LANGUAGES<br/>
- Ruby (gems, tests)
- Javascript (compiler/minimizer, tests)
- ...?

DOCUMENTATION<br/>
- Hah. Poor fools.

RULES<br>
- shared library rules (cc_shared_library)
- install rules (e.g. "make install"-style rule for shared libraries)
- fileset rules (creates symlinked directory with a set of files in it)
- non-cc embed_data rules (e.g. java_embed_data py_embed_data ... similar to cc_embed_data)
- SWIG
- sh_test

PLUGINS<br/>
- Ideally repositories could register a script to modify the BUILD file.
- Script language... go? python?
- Could look like: "register_module" { "name": "my_func", "location": "path/to/my/module.go" } ... "{ "my_func": { ... } }"

FUSE<br/>
- We should not have to explicitly download code not being modified in the current client.
- Directories can be mapped to git/svn/etc repositories on the web, and seamlessly integrated via a readonly mount.
- "third_party" is currently very large (includes boost, amongst others), and we should only have to cache files actually used for compilation locally.
<br/>

GENERATED FILE HANDLING<br/>
- We expand node input files in repobuild's input step. E.g. the '.h' files for a cc_library get checked by repobuild.
- Some of these files might not exist until after dependent rules have run (e.g. protobuf generation).
- We currently do not match against dependent rules' "outs".
- Current hack is to set generated input files as $GEN_DIR/path/to/dep/file, and strict_file_mode_: false.
- Ideally, "path/to/dep/file" would look in "outs" as well as source tree.

DISTRIBUTED BUILD<br/>
- Currently this whole thing generates a makefile.
- Limited distributed build works for c++ using ccache and distcc (configured separately)
- In an ideal world, it would act as a client funneling build commands to remote workers when the local CPU is tapped out.
- Eventually it will require cloud compilers/script runners with VMs (if a service) and a host of crosstools.

PLATFORMS<br/>
- Xcode project files (mac)
- Visual studio project files (windows)
