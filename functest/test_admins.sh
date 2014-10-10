#!/bin/sh

source `dirname $0`/functions

BASEPATH=$1

SGCONFPATH="$BASEPATH/stg/projects/sgconf"

RES=`"$SGCONFPATH/sgconf" -s localhost -p 5555 -u admin -w 123456 --get-admins`

if [ "$?" != "0" ]
then
    printf "Failed to get admins list. Result:\n$RES\n"
    exit 0
fi

LOGINS=`getFields "login" "$RES"`

printf "Logins:\n$LOGINS\n"

NUM=`count "$LOGINS"`

printf -- "--------\n$NUM\n\n"

RES=`"$SGCONFPATH/sgconf" -s localhost -p 5555 -u admin -w 123456 --chg-admin admin --priv 111111111`

if [ "$?" != "0" ]
then
    printf "Failed to update admin's priviledges Result:\n$RES\n"
    exit 0
fi
