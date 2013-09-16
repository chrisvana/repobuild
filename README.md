repobuild
==========

Build tool for my projects

--

Declarative style build system, similar to Google's BUILD file system of old. Other examples:<br/>
<br/>
Gyp: https://code.google.com/p/gyp/<br/>
Buck Build: http://facebook.github.io/buck/<br/>
Selenium build: https://code.google.com/p/selenium/wiki/CrazyFunBuild<br/>
... and there are others that are similar in spirit, e.g.:<br/>
Ant: https://ant.apache.org/

--
TODOs:<br/>
<br/>
LANGUAGES<br/>
- Ruby
- Javascript

RULES<br>
- test rules (e.g. cc_test, java_test, py_test, etc).
- fileset rules (creates symlinked directory with a set of files in it)
- embed_data rules (e.g. cc_embed_data, java_embed_data ... takes a set of files and creates string access in linked code)

PLUGINS<br/>
- Ideally repositories could register a script to modify the BUILD file.
- Script language... go? python?


CODE CLEANUP<br/>
- This is not the prettiest stuff in the world, especially nodes/...


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
