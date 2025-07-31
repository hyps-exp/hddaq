#!/bin/sh

bin_dir=$(dirname `readlink -f $0`)

cd $bin_dir

$bin_dir/message.sh > /dev/null 2> /dev/null &

sleep 1

# enable block register (0th value is dummy)
# en_block=(0xb 0x3 0x3 0x3 0x3 0x0)
en_block=(0xb 0x3 0xf 0x3 0x3 0x0)

for i in $(seq 2 2)
do
    nodeid=`expr 2200 + $i`
    nickname=hul02_scr-`expr 200 + $i`
    dataport=`expr 9200 + $i`
    sitcp_ip=192.168.11.`expr 200 + $i`
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
