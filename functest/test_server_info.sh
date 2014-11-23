#!/bin/sh

source `dirname $0`/functions

BASEPATH=$1

SGCONFPATH="$BASEPATH/stg/projects/sgconf"

printf "Check server info... "

RES=`"$SGCONFPATH/sgconf" -s localhost -p 5555 -u admin -w 123456 --server-info`

if [ "$?" != "0" ]
then
    printf "Error.\n"
    printf "Failed to get admins list. Result:\n$RES\n"
    exit 0
fi

printf "Ok\nResult:\n%s" "$RES"

#LOGINS=`getFields "login" "$RES"`
#
#NUM=`count "$LOGINS"`
#
#if [ "$NUM" != "1" ]
#then
#    printf "Failed.\n"
#    printf "Admin list should have exactly one entry.\n"
#    printf "Logins:\n$LOGINS\n"
#    printf -- "--------\n$NUM\n\n"
#    exit 0
#fi
