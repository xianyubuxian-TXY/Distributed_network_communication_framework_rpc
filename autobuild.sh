#!/bin/bash

set -e

rm -rf `pwd`/build/*
cd `pwd`/build &&
    cmake .. &&
    make
cd ..
# 项目编译生成的libmprpc静态库在lib下，需要相应的头文件，所以将`pwd`/src/include拷贝到`pwd`/lib中
cp -r `pwd`/src/include `pwd`/lib