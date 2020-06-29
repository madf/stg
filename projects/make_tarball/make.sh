#!/bin/sh

if [ "$1" != "" ]
then
    SRC_DIR="$1"
else
    PREFIX="stg-2.4"
    SRC_DIR=$(date "+${PREFIX}-%Y.%m.%d-%H.%M.%S")
fi

ARC_NAME=$SRC_DIR.tar.gz

git clone git@stg.codes:stg.git -b stg-2.409 $SRC_DIR

if [ $? != 0 ]
then
    echo "Failed to clone repository"
    exit -1
fi

rm -rf $SRC_DIR/.git
rm -f $SRC_DIR/.gitignore
rm -f $SRC_DIR/.travis.yml
rm -r $SRC_DIR/projects/make_tarball
rm -r $SRC_DIR/projects/sgauthstress
rm -r $SRC_DIR/projects/rlm_stg
rm -r $SRC_DIR/projects/stargazer/plugins/authorization/stress
rm $SRC_DIR/doc/help.odt

tar -zcf $ARC_NAME $SRC_DIR
