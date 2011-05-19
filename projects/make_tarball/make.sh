#!/bin/sh

SRC_DIR=stg-2.4-`date "+%Y.%m.%d-%H.%M.%S"`
ARC_NAME=$SRC_DIR.tar.gz

git clone git://gitorious.org/stg/stg.git $SRC_DIR

if [ $? != 0 ]
then
    echo "Failed to clone repository"
    exit -1
fi

rm -rf $SRC_DIR/.git
rm -f $SRC_DIR/.gitignore
rm -r $SRC_DIR/projects/make_tarball
rm -r $SRC_DIR/projects/traffcounter
rm -r $SRC_DIR/projects/sgauthstress
rm -r $SRC_DIR/projects/stargazer/plugins/authorization/stress
rm -r $SRC_DIR/projects/stargazer/plugins/configuration/sgconfig2
rm -r $SRC_DIR/projects/stargazer/plugins/configuration/sgconfig-ng
rm -r $SRC_DIR/projects/stargazer/plugins/configuration/xrconfig
rm -r $SRC_DIR/projects/stargazer/plugins/other/userstat
rm -r $SRC_DIR/projects/stargazer/plugins/store/db
rm -r $SRC_DIR/doc/book
rm $SRC_DIR/doc/help.odt

make -C $SRC_DIR/doc/xmlrpc

tar -zcf $ARC_NAME $SRC_DIR
