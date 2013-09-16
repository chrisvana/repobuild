repobuild
==========

Build tool for my projects.<br/>
<br/>
Functional, but work in progress.<br/>

--

Declarative style build system, similar to Google's BUILD file system of old.<br/>
<br/>
Similar build systems:
- Gyp: https://code.google.com/p/gyp/<br/>
- Buck Build: http://facebook.github.io/buck/<br/>
- Selenium build: https://code.google.com/p/selenium/wiki/CrazyFunBuild<br/>
- (sort of, in spirit) Ant: https://ant.apache.org/<br/>
<br/>
Motivation:<br/>
- Procedural build tools (e.g. make, and most everything else) tend to devolve into not-particular-modular messy rules, and are hard to read/modify/inherit.
- Google's BUILD system works pretty well (ask other ex-Googlers)
- A lot of open source libraries assume pre-installation on a platform. I really did not like that model, so everything now goes into "third_party" with its own BUILD tool (minus a few examples like "-lzlib" floating around).
- By not-preinstalling everything, we can keep repositories on the web, and pretend they exist in some "readonly" directory (work in progres!). This means you can "git clone" a tiny repository, then modify and compile it even if it depends on a bunch of other stuff (which magically gets pulled out of "readonly"). See TODO below.

--

What should you do now?<br/>
- Look in testdata/BUILD, or repobuild/BUILD for some examples.<br/>
- Rather than procedurally specify make rules, BUILD files auto-generate a make file by expanding dependencies amongst components.
- Look at additional libraries are in "third_party", which you can add submodules/forks to if you like.

--
TODOs:<br/>
<br/>
LANGUAGES<br/>
- Ruby
- Javascript

DOCUMENTATION<br/>
- Hah. Poor sucker.

RULES<br>
- test rules (e.g. cc_test, java_test, py_test, etc).
- shared library rules (cc_shared_library)
- install rules (e.g. "make install"-style rule for shared libraries)
- fileset rules (creates symlinked directory with a set of files in it)
- embed_data rules (e.g. cc_embed_data, java_embed_data ... takes a set of files and creates string access in linked code)

PLUGINS<br/>
- Ideally repositories could register a script to modify the BUILD file.
- Script language... go? python?
- Could look like: "register_module" { "name": "my_func", "location": "path/to/my/module.go" } ... "{ "my_func": { ... } }"

CODE CLEANUP<br/>
- This is not the prettiest stuff in the world, especially nodes/...

FILE HANDLING<br/>
- We expand files in repobuild's input step.
- We currently do not match against dependent rules' "outs".
- Current hack is to set file as $GEN_DIR/path/to/dep/file, and strict_file_mode_: false.
- Ideally, "path/tp/dep/file" would look in "outs" as well as source tree.

FUSE<br/>
- We should not have to explicitly download code not being modified in the current client.
- Directories can be mapped to git/svn/etc repositories on the web, and seamlessly integrated via a readonly mount.
- "third_party" is currently very large (includes boost, amongst others), and we should only have to cache files actually used for compilation locally.
<br/>

DISTRIBUTED BUILD<br/>
- Currently this whole thing generates a makefile.
- Limited distributed build works for c++ using ccache and distcc (configured separately)
- In an ideal world, it would act as a client funneling build commands to remote workers when the local CPU is tapped out.
- Eventually it will require cloud compilers/script runners with VMs (if a service) and a host of crosstools.
