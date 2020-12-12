#!/bin/sh

bin_dir=$(dirname `readlink -f $0`)

cd $bin_dir

$bin_dir/message.sh > /dev/null 2> /dev/null &

sleep 1

nodeid=2001
nickname=hul_hbx_scr
dataport=9001 
sitcp_ip=192.168.12.11
master=--master
en_block=0x3

$bin_dir/frontend.sh \
    $nickname \
    $nodeid \
    $dataport \
    $sitcp_ip \
    $master \
    $en_block \
    >/dev/null 2>/dev/null &
