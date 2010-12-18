#!/bin/sh

SRC_DIR=stg-2.4-`date "+%Y.%m.%d-%H.%M.%S"`
ARC_NAME=$SRC_DIR.tar.gz

git clone git://madf.dyndns.org/stg.git $SRC_DIR

if [ $? != 0 ]
then
    echo "Failed to clone repository"
    exit -1
fi

rm -rf $SRC_DIR/.git
rm -r $SRC_DIR/projects/make_tarball
rm -r $SRC_DIR/projects/traffcounter
rm -r $SRC_DIR/projects/stargazer/plugins/other/userstat
rm -r $SRC_DIR/projects/stargazer/plugins/authorization/stress
rm -r $SRC_DIR/projects/stargazer/plugins/store/db

tar -zcf $ARC_NAME $SRC_DIR
