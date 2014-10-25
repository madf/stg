#!/bin/sh

MKTEMP=/usr/bin/mktemp
RM=/bin/rm

printf "Creating temporary dir... "
DIR=`$MKTEMP -d 2> /dev/null || $MKTEMP -d -t stg`

if [ "$DIR" == "" ]
then
    printf "Failed.\nTemporary dir is empty.\n"
    exit -1
fi

if [ ! -d "$DIR" ]
then
    printf "Failed.\nTemporary dir '$DIR' does not exist or not a directory.\n"
    exit -1
fi

LOGFILE=`date "+%Y-%m-%d-%H%M%S.log"`

printf "Ok. Working dir: $DIR\nCloning... "
./clone.sh "$DIR" >> "$LOGFILE" 2>&1
if [ "$?" != "0" ]
then
    printf "Failed.\n"
    exit -1
else
    printf "Ok.\nBuilding... "
fi
./build.sh "$DIR" >> "$LOGFILE" 2>&1
if [ "$?" != "0" ]
then
    printf "Failed.\n"
    exit -1
else
    printf "Ok.\n"
fi

./test.sh "$DIR" # >> "$LOGFILE" 2>&1

printf "Cleaning up... "

$RM -rf $DIR
if [ "$?" != "0" ]
then
    printf "Failed.\n"
    exit -1
else
    printf "Ok.\n"
fi
