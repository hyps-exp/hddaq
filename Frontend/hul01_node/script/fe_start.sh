#!/bin/sh

bin_dir=$(dirname `readlink -f $0`)

cd $bin_dir

$bin_dir/message.sh > /dev/null 2> /dev/null &

sleep 1

nodeid=$((0x601))
nickname=hul01
dataport=9000
sitcp_ip=192.168.10.61

$bin_dir/frontend.sh \
    $nickname \
    $nodeid \
    $dataport \
    $sitcp_ip \
    >/dev/null 2>/dev/null &
