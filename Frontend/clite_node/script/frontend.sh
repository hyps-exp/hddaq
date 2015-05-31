#!/bin/sh

while true
do
    ./fecopperlite fptdc_init --tcp-host=$1 --udp-host=$2 --nickname=$3 --nodeid=$4 --data-port=$5
    sleep 1
done