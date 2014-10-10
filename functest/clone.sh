#!/bin/sh

BASEPATH=$1
ARCHIVE=$2
CURPATH=`pwd`
GIT=/usr/bin/git
TAR=/bin/tar
RM=/bin/rm

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

#$GIT clone "https://gitorious.org/stg/stg.git" "$BASEPATH/stg"
$GIT clone "../" "$BASEPATH/stg"

if [ "$ARCHIVE" != "" ]
then
    $RM -rf "$BASEPATH/stg/.git"

    $TAR -C "$BASEPATH" -jcf "$BASEPATH/stg.tar" "stg"
fi
