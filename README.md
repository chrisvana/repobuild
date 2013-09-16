repobuild
==========

Build tool for my projects.<br/>
(apparently this is what happens when I get bored)<br/>
<br/>
Functional, but a work in progress.<br/>


--
Summary

Declarative style build system, similar to Google's BUILD file system of old.<br/>
<br/>
Similar build systems that did not quite do whawt I wanted:
- Gyp: https://code.google.com/p/gyp/<br/>
- Buck Build: http://facebook.github.io/buck/<br/>
- Selenium build: https://code.google.com/p/selenium/wiki/CrazyFunBuild<br/>
- (sort of, in spirit) Ant: https://ant.apache.org/<br/>
<br/>

--

--
Motivation
Coming out of Google after most of a decade, I felt like existing open source build systems had a lot of issues.
 
Declarative/Modular:
- Procedural build tools (e.g. make, and most everything else) tend to devolve into not-particular-modular messy rules, and are hard to read/modify/inherit.
- Current open source libraries are not easily interconnected, making the relative project size obtainable rather limited. Which is silly!
- By the way, being able to easily build on top of thousands of components is also why Google's libraries are rarely open source: They depened on too many other components that are also not open sourced (even the most simple libraries can often depend on, say, 1M lines of code in 100 different other projects).
- Google's BUILD system works pretty well for large modular development (ask other ex-Googlers), and I did not want to re-invent the wheel.

No pre-installation:
- A lot of open source libraries assume pre-installation on a platform. I found this to be a huge pain.
- Everything now goes into "third_party" with its own BUILD tool (minus a few examples like "-lzlib" floating around).
- By not-preinstalling everything, we can keep repositories on the web, and pretend they exist in some "readonly" directory (work in progres, see TODO below!).
- "Readonly" means you can "git clone" a tiny repository, then modify and compile it even if it depends on a 100 other components (which magically get pulled out of "readonly" as needed). This makes development a lot easier and encourages more modularity.

--

What should you do now?<br/>
- Look in testdata/BUILD, or repobuild/BUILD for some examples.<br/>
- Rather than procedurally specify make rules, BUILD files auto-generate a make file by expanding dependencies amongst components.
- Look at additional libraries are in "third_party".
- Start playing with your own project.
- You can add submodules/forks to "third_party" if you like.

Forum:
- https://groups.google.com/forum/#!forum/repobuild

--
TODOs:<br/>
<br/>
LANGUAGES<br/>
- Ruby (gems, tests)
- Javascript (compiler/minimizer, tests)
- ...?

DOCUMENTATION<br/>
- Hah. Poor fools.

RULES<br>
- test rules (e.g. cc_test, java_test, py_test, etc).
- shared library rules (cc_shared_library)
- install rules (e.g. "make install"-style rule for shared libraries)
- fileset rules (creates symlinked directory with a set of files in it)
- SWIG
- embed_data rules (e.g. cc_embed_data, java_embed_data py_embed_data ... takes a set of files and creates string access in linked code)

PLUGINS<br/>
- Ideally repositories could register a script to modify the BUILD file.
- Script language... go? python?
- Could look like: "register_module" { "name": "my_func", "location": "path/to/my/module.go" } ... "{ "my_func": { ... } }"

CODE CLEANUP<br/>
- This is not the prettiest stuff in the world, especially nodes/...

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
- Ideally, "path/tp/dep/file" would look in "outs" as well as source tree.

DISTRIBUTED BUILD<br/>
- Currently this whole thing generates a makefile.
- Limited distributed build works for c++ using ccache and distcc (configured separately)
- In an ideal world, it would act as a client funneling build commands to remote workers when the local CPU is tapped out.
- Eventually it will require cloud compilers/script runners with VMs (if a service) and a host of crosstools.
