#!/bin/sh

source `dirname $0`/functions

BASEPATH=$1

SGCONFPATH="$BASEPATH/stg/projects/sgconf"

printf "Check initial admin list... "

RES=`"$SGCONFPATH/sgconf" -s localhost -p 5555 -u admin -w 123456 --get-admins`

if [ "$?" != "0" ]
then
    printf "Error.\n"
    printf "Failed to get admins list. Result:\n$RES\n"
    exit 0
fi

LOGINS=`getFields "login" "$RES"`

NUM=`count "$LOGINS"`

if [ "$NUM" != "1" ]
then
    printf "Failed.\n"
    printf "Admin list should have exactly one entry.\n"
    printf "Logins:\n$LOGINS\n"
    printf -- "--------\n$NUM\n\n"
    exit 0
fi

printf "Ok.\nCheck updating admin permissions... "

RES=`"$SGCONFPATH/sgconf" -s localhost -p 5555 -u admin -w 123456 --chg-admin admin --priv 111111111`

if [ "$?" != "0" ]
then
    printf "Error\n"
    printf "Failed to update admin's priviledges Result:\n$RES\n"
    exit 0
fi

printf "Ok.\n"
