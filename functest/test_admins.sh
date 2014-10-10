#!/bin/sh

BASEPATH=$1

SGCONFPATH="$BASEPATH/stg/projects/sgconf"

RES=`"$SGCONFPATH/sgconf" -s localhost -p 5555 -u admin -w 123456 --get-admins`

if [ "$?" != "0" ]
then
    printf "Failed to get admins list. Result:\n$RES\n"
    exit 0
fi

printf "Got admins list:\n"

LOGINS=""
OLDIFS=$IFS
IFS=$(printf "\n")
for LINE in $RES
do
    printf -- "$LINE\n"
    LOGIN=`printf "$LINE" | grep login`
    if [ "$?" == "0" ]
    then
        LOGINS="$LOGINS\n"`printf "$LOGIN" | cut -d: -f2 | sed -e 's/^ *//' -e 's/ *$//'`
    fi
done
IFS=$OLDIFS

printf "Logins:\n$LOGINS\n"

NUM=`printf "$LOGINS" | wc -l`

printf -- "--------\n$NUM\n\n"
