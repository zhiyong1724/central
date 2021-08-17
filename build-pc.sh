#!/bin/bash
export PLATFORM=PC
echo "PLATFORM=$PLATFORM"

export FLATFORM_DIR=platform/linux
echo "FLATFORM_DIR=$FLATFORM_DIR"

export BUILD_TYPE=Debug
#export BUILD_TYPE=Release
echo "BUILD_TYPE=$BUILD_TYPE"

export PATH=$PATH
echo "PATH=$PATH"

export CONFIG_HOST=
echo "CONFIG_HOST=$CONFIG_HOST"

export CC=gcc
echo "CC=$CC"

export CXX=g++
echo "CXX=$CXX"

export AR=ar
echo "AR=$AR"

export LD=ld
echo "LD=$LD"

export DUMP=objdump
echo "DUMP=$DUMP"

export INCLUDES="./third/lvgl/"
echo "INCLUDES=$INCLUDES"

export LIBS_DIR="-L./third/lvgl/"
echo "LIBS_DIR=$LIBS_DIR"

export LIBS="-lpthread -lX11 -llvgl"
echo "LIBS=$LIBS"

export COMPILE_FLAGS="-g -m32"
echo "COMPILE_FLAGS=$COMPILE_FLAGS"

make