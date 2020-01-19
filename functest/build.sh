#!/bin/sh

BASEPATH=$1

if [ "$BASEPATH" == "" ]
then
    printf "Usage: $0 <path>\n"
    exit -1
fi

if [ ! -d "$BASEPATH" ]
then
    printf "Path '$BASEPATH' does not exist or not a directory.\n"
    exit -1
fi

cd "$BASEPATH/stg"
mkdir build
cd build
cmake -DBUILD_STG=ON -DBUILD_SGCONF=ON ..
make
