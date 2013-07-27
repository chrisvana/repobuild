#!/bin/bash

COMPILER=clang++
CC="$COMPILER -std=c++11 -stdlib=libc++ -pthread -DUSE_CXX0X"

function run() {
    while read line; do
        obj_dir=objs/$(dirname $line)
        mkdir -p $obj_dir
        $CC -c $@ $line -o $obj_dir/$(basename $line).o 
    done
}

#ls common/strings/re2/*/*.cc | grep -v "test" | grep -v "benchmark" | run -Icommon/strings/re2
#ls common/*/*.cc | run -Icommon -Icommon/strings/re2
ls env/*.cc | run -Icommon -I.
ls json/src/lib_json/*.cpp | run -Ijson/include
ls reader/*.cc | run -Icommon -I.
ls generator/*.cc | run -Icommon -I.
ls repobuild.cc | run -Icommon -I.

files=$(find objs | grep '\.o')
$CC $files -o repobuild
