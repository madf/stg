#!/bin/sh

source `dirname $0`/functions

BASEPATH=$1

SGCONFPATH="$BASEPATH/stg/projects/sgconf"

printf "Check initial service list... "

RES=`"$SGCONFPATH/sgconf" -s localhost -p 5555 -u admin -w 123456 --get-services`

if [ "$?" != "0" ]
then
    printf "Error.\n"
    printf "Failed to get services list. Result:\n$RES\n"
    exit 0
fi

NAMES=`getFields "name" "$RES"`

NUM=`count "$NAMES"`

if [ "$NUM" != "0" ]
then
    printf "Failed.\n"
    printf "Services list should be empty.\n"
    printf "Names:\n$NAMES\n"
    printf -- "--------\n$NUM\n\n"
    exit 0
fi

printf "Ok.\nCheck adding services... "

RES=`"$SGCONFPATH/sgconf" -s localhost -p 5555 -u admin -w 123456 --add-service test`

if [ "$?" != "0" ]
then
    printf "Error.\n"
    printf "Failed to add new service. Result:\n$RES\n"
    exit 0
fi

printf "Ok.\nCheck new service list... "

RES=`"$SGCONFPATH/sgconf" -s localhost -p 5555 -u admin -w 123456 --get-services`

if [ "$?" != "0" ]
then
    printf "Error.\n"
    printf "Failed to get services list. Result:\n$RES\n"
    exit 0
fi

NAMES=`getFields "name" "$RES"`

NUM=`count "$NAMES"`

if [ "$NUM" != "1" ]
then
    printf "Failed.\n"
    printf "Services list should have exactly one entry.\n"
    printf "Names:\n$NAMES\n"
    printf -- "--------\n$NUM\n\n"
    exit 0
fi

printf "Ok.\nCheck deletion exisiting service... "

RES=`"$SGCONFPATH/sgconf" -s localhost -p 5555 -u admin -w 123456 --del-service test`

if [ "$?" != "0" ]
then
    printf "Error.\n"
    printf "Failed to delete a service. Result:\n$RES\n"
    exit 0
fi

printf "Ok.\nCheck new service list... "

RES=`"$SGCONFPATH/sgconf" -s localhost -p 5555 -u admin -w 123456 --get-services`

if [ "$?" != "0" ]
then
    printf "Error.\n"
    printf "Failed to get services list. Result:\n$RES\n"
    exit 0
fi

NAMES=`getFields "name" "$RES"`

NUM=`count "$NAMES"`

if [ "$NUM" != "0" ]
then
    printf "Failed.\n"
    printf "Services list should be empty.\n"
    printf "Names:\n$NAMES\n"
    printf -- "--------\n$NUM\n\n"
    exit 0
fi

printf "Ok.\n"
