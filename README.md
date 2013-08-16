repobuild
==========

Build tool for my projects

--

Declarative style build system, similar to google's BUILD file system. Other examples:<br/>
<br/>
Gyp: https://code.google.com/p/gyp/
Buck Build: http://facebook.github.io/buck/<br/>
Selenium build: https://code.google.com/p/selenium/wiki/CrazyFunBuild<br/>
... and there are others.


--
TODOs

FUSE
Currently this uses some ugly -I patterns and directory structure symlinks. Instead, we should have a FUSE based filesystem that powers the file structure:
1) .gen-files/ dir
2) .gen-obj/ dir
3) local paths to sources
4) remote (git) paths to sources

import "foo/bar/baz.go" => Checks gen-files, checks local paths, checks remote git paths.
Remote paths will need some configuring.

MAKE
Currently this whole thing generates a makefile. In an ideal world, it would also wrap a lot of the build system (pump, distcc, ccache, etc) to make that seemless.

LINKING
All of the object files get linked at the same time. Probably significantly better to create merged, shared object files.
