repobuild
==========

###### Summary
- Repobuild is a declarative style build system
- Similar to Google's BUILD file system of old (gconfig + make)
- Repobulid is a pre-processor that generates a Makefile and fetches missing files via git.

###### Status
- Functional for Mac and Linux, but a work in progress.
- In beta, some rules may change.

###### Why?
- [Motivation](https://github.com/chrisvana/repobuild/wiki/Motivation) behind Repobuild
- [Similar build systems](https://github.com/chrisvana/repobuild/wiki/Similar-Build-Systems) to Repobuild

###### Current Languages:
- C++
- Python
- Java
- Go
- _TODO:_ Ruby, Javascript (compiler/minimizer), Scala, etc.

###### Current rules:
- See [rules](https://github.com/chrisvana/repobuild/wiki/Rules)

--
###### Dependencies
- git
- gcc or clang (requires c++11)
- make
- some rule types require cmake, python, go, java

###### Building the tool
```
# Build it:
$ git clone https://github.com/chrisvana/repobuild.git
$ cd repobuild
$ make -j8 repobuild

# Install somewhere in your path
$ # Example: sudo cp bin/repobuild  #/... somewhere in your path, like /usr/bin
$ sudo cp bin/repobuild /usr/bin/repobuild

# Usage:
$ repobuild --helpshort
$ ...

# Try it out on the testdata (builds in protocol buffers and other fun stuff):
$ repobuild "testdata:java_main"
$ make -j8
$ ./java_main

```

###### What should you do now?
- Try a [tutorial](https://github.com/chrisvana/repobuild/wiki/Repobuild-Cpp-Tutorial)
- Look at some [examples](https://github.com/chrisvana/repobuild/wiki/Examples)
- Start playing with your own project.
- Add submodules to "third_party".
- Forum: https://groups.google.com/forum/#!forum/repobuild

###### Why is this built using C++?<br/>
Actually this started in Go, and there is no good reason for C++. The first open source code I needed to use it on was in C++, and being able to build the tool using the tool appealed to the geek in me.

###### TODOs
https://github.com/chrisvana/repobuild/wiki/TODOs

###### Tips
https://github.com/chrisvana/repobuild/wiki/Tips
