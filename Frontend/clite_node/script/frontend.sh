#!/bin/sh

bin_dir=$(cd $(dirname $0); pwd -P)/../bin
frontend=clite_frontend

while true
do
    $bin_dir/$frontend \
	fptdc_init \
	--tcp-host=$1 \
	--udp-host=$2 \
	--nickname=$3 \
	--nodeid=$4   \
	--data-port=$5

    sleep 1
done