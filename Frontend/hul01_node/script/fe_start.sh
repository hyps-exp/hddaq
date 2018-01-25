#!/bin/sh

bin_dir=$(dirname `readlink -f $0`)

cd $bin_dir

$bin_dir/message.sh > /dev/null 2> /dev/null &

sleep 1

for i in $(seq 1 2)
do
    nodeid=`expr $((0x600)) + $i`
    nickname=hul01scr-`expr + $i`
    dataport=`expr 8999 + $i` 
    sitcp_ip=192.168.10.`expr 60 + $i`

    $bin_dir/frontend.sh \
	$nickname \
	$nodeid \
	$dataport \
	$sitcp_ip \
	>/dev/null 2>/dev/null &
done