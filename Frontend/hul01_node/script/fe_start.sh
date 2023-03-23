#!/bin/sh

bin_dir=$(dirname `readlink -f $0`)

cd $bin_dir

$bin_dir/message.sh > /dev/null 2> /dev/null &

sleep 1

# enable block register (0th value is dummy)
en_block=(0x0, 0xb, 0x1, 0x3)

for i in $(seq 1 2)
#for i in $(seq 2 2)
do
    nodeid=`expr $((0x600)) + $i`
    nickname=hul01scr-`expr + $i`
    dataport=`expr 8999 + $i`
    sitcp_ip=192.168.10.`expr 60 + $i`
    master=--master

    if [ $i -eq 3 ]; then
	master=--slave
    fi

    $bin_dir/frontend.sh \
	$nickname \
	$nodeid \
	$dataport \
	$sitcp_ip \
	$master \
	${en_block[$i]} \
	>/dev/null 2>/dev/null &
done
