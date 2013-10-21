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
- A small side project, so turnaround might be slow.

###### Why?
- Initially a learning project to make open source easier to integrate.
- [Motivation](https://github.com/chrisvana/repobuild/wiki/Motivation) behind Repobuild
- Neat features:
  - Sub-module dependency initialization: see [here](https://github.com/chrisvana/repobuild/wiki/Sub-Module-Handling).
  - Inteded to wrap existing build tools: No project rewrites required.
  - Generates a Makefile: Clients of your code can just run "make" without repobuild installed.
  - Plugins: Allows custom scripts to rewrite BUILD files during execution ([simple example](https://github.com/chrisvana/repobuild/wiki/Plugins))
- A lot of other bulid tools have better functionality (and years of development). This is not intended to replace them outright.
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
```

--
###### Usage
```
# Basic help:
$ repobuild --helpshort
repobuild: 

  To generate makefile:
     repobuild "path/to/dir:target" [--makefile=Makefile]

  To build:
     make [-j8] [target]

  To run:
     ./.gen-obj/path/to/target
         or
     ./target

  Flags from repobuild/repobuild.cc:
    -makefile (Name of makefile output.) type: string default: "Makefile"

# To set up makefile:
$ repobuild "//path/to/my/dir:target_name" "//path/to/other/dir:target_2"
....
Generating Makefile

# Make options:
# 1) Builds all binaries, libraries, etc:
$ make

# 2) Build specific library
$ make path/to/my/dir/target_name

# 3) Builds/runs tests:
$ make tests

# 4) Installs binaries and shared libraries
$ make install
```

*Concrete example*
```
# Try it out on the testdata (builds in protocol buffers and other fun stuff):
$ repobuild "testdata:java_main"
...
$ make -j8
...
$ ./java_main
```

###### What should you do now?
- Try a [tutorial](https://github.com/chrisvana/repobuild/wiki/Examples#tutorials)
- Look at some other [examples](https://github.com/chrisvana/repobuild/wiki/Examples)
- Start playing with your own project.
- Add open source projects to ["third_party"](https://github.com/chrisvana/third_party).
- Write a [plugin](https://github.com/chrisvana/repobuild/wiki/Plugins), and optionally add to [//third_party/plugins](https://github.com/chrisvana/repo_plugins)
- Forum (questions, bugs, hate mail, etc): https://groups.google.com/forum/#!forum/repobuild

###### Why is this built using C++?<br/>
Actually this started in Go, and there is no good reason for C++. The first open source code I needed to use it on was in C++, and being able to build the tool using the tool appealed to the geek in me.

###### TODOs
https://github.com/chrisvana/repobuild/wiki/TODOs

###### Tips
https://github.com/chrisvana/repobuild/wiki/Tips
