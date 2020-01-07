#!/bin/bash

int_iface=eth1

iptables -t mangle --flush

tc qdisc add dev $int_iface root handle 1: htb
tc class add dev $int_iface parent 1: classid 1:1 htb rate 100mbit ceil 100mbit burst 200k

