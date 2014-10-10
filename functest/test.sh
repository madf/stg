#!/bin/sh

BASEPATH=$1

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

STGPATH="$BASEPATH/stg/projects/stargazer"

cp "stuff/stargazer-files.conf" "$STGPATH/stargazer.conf"
cp "stuff/rules" "$STGPATH/"
cp "stuff/OnConnect" "$STGPATH/"
cp "stuff/OnDisconnect" "$STGPATH/"
cp "stuff/OnChange" "$STGPATH/"
cp -R "stuff/db-stub" "$STGPATH/db"

sed -i "s|-STG-PATH-|$STGPATH|g" "$STGPATH/stargazer.conf"

LOGFILE=`pwd`"/"`date "+%Y-%m-%d-%H%M%S.console.log"`

cd "$STGPATH"

"$STGPATH/stargazer" "$STGPATH" >> "$LOGFILE" 2>&1 &

COUNT=""
while true
do
    grep "Stg started successfully" "$STGPATH/stargazer.log" > /dev/null 2> /dev/null
    if [ "$?" == "0" ]
    then
        break
    fi
    COUNT="$COUNT."
    if [ "$COUNT" == "....." ]
    then
        printf "Failed to start stg in 5 sec.\n"
        exit -1
    fi
    sleep 1
done

PID=`cat "$STGPATH/stargazer.pid"`
printf "Started with pid $PID\n"

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
    LOGIN=`echo $LINE | grep login`
    if [ "$?" == "0" ]
    then
        LOGINS="$LOGINS\n"`echo $LOGIN | cut -d: -f2 | sed -e 's/^ *//' -e 's/ *$//'`
    fi
done
IFS=$OLDIFS

printf "Logins:\n$LOGINS\n"

NUM=`echo $LOGINS | wc -l`

printf -- "--------\n$NUM\n\n"

printf "Stopping...\n"
kill $PID

COUNT=""
while true
do
    grep "Stg stopped successfully" "$STGPATH/stargazer.log" > /dev/null 2> /dev/null
    if [ "$?" == "0" ]
    then
        break
    fi
    COUNT="$COUNT."
    if [ "$COUNT" == "....." ]
    then
        printf "Failed to stop stg in 5 sec.\n"
        exit -1
    fi
    sleep 1
done

printf "Stopped.\n"
