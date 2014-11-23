#!/bin/sh

BASEPATH=$1

source `dirname $0`/functions

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

GROUP=root
groups | grep root > /dev/null 2> /dev/null
if [ "$?" != "0" ]
then
    groups | grep wheel > /dev/null 2> /dev/null
    if [ "$?" != "0" ]
    then
        printf "Can't find neither 'root' nor 'wheel' group.\n"
        exit -1
    fi
    GROUP=wheel
fi

subst "-STG-PATH-" "$STGPATH" "$STGPATH/stargazer.conf"
subst "-STG-GROUP-" "$GROUP" "$STGPATH/stargazer.conf"

CURPATH=`pwd`
LOGFILE="$CURPATH/"`date "+%Y-%m-%d-%H%M%S.console.log"`

cd "$STGPATH"

printf "Starting Stargazer... "

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

printf "\nTesting server info:\n"
"$CURPATH/test_server_info.sh" "$BASEPATH"
printf "\nTesting admins:\n"
"$CURPATH/test_admins.sh" "$BASEPATH"
printf "\nTesting services:\n"
"$CURPATH/test_services.sh" "$BASEPATH"
printf "\n"

printf "Stopping... "
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
