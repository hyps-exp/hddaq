#!/bin/sh

bin_dir=$(dirname `readlink -f $0`)

cd $bin_dir

$bin_dir/message.sh > /dev/null 2> /dev/null &

sleep 1

# enable block register (0th value is dummy)
en_block=(0x3)

for i in $(seq 0 0)
do
    nodeid=`expr $((0x600)) + $i`
    nickname=hul04scr-`expr 1 + $i`
    dataport=`expr 9000 + $i`
    sitcp_ip=192.168.10.`expr 100 + $i`
    master=--master

    # if [ $i != 0 ]; then
    # 	master=--slave
    # fi

    $bin_dir/frontend.sh \
	$nickname \
	$nodeid \
	$dataport \
	$sitcp_ip \
	$master \
	${en_block[$i]} \
	>/dev/null 2>/dev/null &
done
