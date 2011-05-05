#!/bin/bash

users=`cat users.1000`
hi=0
lo=0
for user in $users
do
    cp -R /var/stargazer/users/test /var/stargazer/users/$user
    if (( $lo > 254 ))
    then
        lo=0
        let hi++
    fi
    let lo++
    ip="10.22.$hi.$lo"
    ip addr add $ip dev eth0
done
