#!/bin/sh

bin_dir=$(dirname `readlink -f $0`)

cd $bin_dir

$bin_dir/message.sh > /dev/null 2> /dev/null &

sleep 1

$bin_dir/mass_trigger_start

sleep 1

nodeid=$((0x603))
nickname=hul_mst
dataport=9001
sitcp_ip=192.168.10.63

$bin_dir/frontend.sh \
    $nickname \
    $nodeid \
    $dataport \
    $sitcp_ip \
    >/dev/null 2>/dev/null &
