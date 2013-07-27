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

ls common/strings/re2/*.cc common/*/*.cc | run -I.
ls env/*.cc | run -I.
ls json/src/lib_json/*.cpp | run -Ijson/include
ls reader/*.cc | run -I.
ls nodes/*.cc | run -I.
ls generator/*.cc | run -I.
ls repobuild.cc | run -I.

#
files=$(find objs | grep '\.o')
$CC $files -o repobuild
