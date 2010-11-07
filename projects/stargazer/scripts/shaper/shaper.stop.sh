#!/bin/bash

int_iface=eth1

#iptables -t mangle --flush

tc qdisc del dev $int_iface root handle 1: htb

