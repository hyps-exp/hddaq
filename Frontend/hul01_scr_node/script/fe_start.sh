#!/bin/sh

bin_dir=$(dirname `readlink -f $0`)

cd $bin_dir

$bin_dir/message.sh > /dev/null 2> /dev/null &

sleep 1

# enable block register (0th value is dummy)
en_block=(0xb 0x3 0x3 0x3 0x3 0x0)

for i in $(seq 0 1) # for debug
# for i in $(seq 0 1)
do
    nodeid=`expr 3001 + $i`
    nickname=hul01_scr-`expr 1 + $i`
    dataport=`expr 9201 + $i`
    sitcp_ip=192.168.11.`expr 201 + $i`
    master=--master
    # master=--slave
    # if [ $i -eq 200 ]; then
    # 	master=--master
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
