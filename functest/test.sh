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

CURPATH=`pwd`
LOGFILE="$CURPATH/"`date "+%Y-%m-%d-%H%M%S.console.log"`

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

"$CURPATH/test_admins.sh" "$BASEPATH"
"$CURPATH/test_services.sh" "$BASEPATH"

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
