repobuild
==========

###### Summary
Repobuild is a declarative style build system.
- Similar to Google's BUILD file system of old (gconfig + make)
- Generates a Makefile for portability
- Fetches missing files from git automatically

###### Status
- Functional for Mac and Linux.
- In beta, some rules may change.

###### Why?
- Initially a learning project
  - A lot of other bulid tools have better functionality (and years of development)
  - This is not intended to replace other tools outright.
- [Motivation](https://github.com/chrisvana/repobuild/wiki/Motivation) behind Repobuild
- [Similar build systems](https://github.com/chrisvana/repobuild/wiki/Similar-Build-Systems) to Repobuild
- Neat features:
  - Sub-module dependency initialization: see [here](https://github.com/chrisvana/repobuild/wiki/Sub-Module-Handling).
  - Wraps existing common build tools: No project rewrites required.
  - Generates a Makefile: Clients of your code can just run "make" without repobuild installed.

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
- gcc or clang (recent version, requires c++11)
- make
- some rule types require python, go, java, cmake, etc.

###### Building the tool
```
# Build it:
$ git clone https://github.com/chrisvana/repobuild.git
$ cd repobuild
$ make -j8 repobuild

# Install (default is /usr/local/bin)
$ sudo make install

# Usage:
$ repobuild --helpshort
$ ...

# Try it out on the testdata (builds in protocol buffers and other fun stuff):
$ repobuild "testdata:java_main"
...
$ make -j8
...
$ ./java_main

```

###### What should you do now?
- Try a [tutorial](https://github.com/chrisvana/repobuild/wiki/Repobuild-Cpp-Tutorial)
- Look at some [examples](https://github.com/chrisvana/repobuild/wiki/Examples)
- Start playing with your own project.
- Add submodules to ["third_party"](https://github.com/chrisvana/third_party).
- Forum (questions, bugs, hate mail, etc): https://groups.google.com/forum/#!forum/repobuild

###### Why is this built using C++?<br/>
Actually this started in Go, and there is no good reason for C++. The first open source code I needed to use it on was in C++, and being able to build the tool using the tool appealed to the geek in me.

###### TODOs
https://github.com/chrisvana/repobuild/wiki/TODOs

###### Tips
https://github.com/chrisvana/repobuild/wiki/Tips
