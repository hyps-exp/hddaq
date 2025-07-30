#!/bin/sh

bin_dir=$(dirname `readlink -f $0`)

cd $bin_dir

#$bin_dir/message.sh > /dev/null 2> /dev/null &
./message.sh > /dev/null 2> /dev/null &

sleep 1

for i in $(seq 1 9)
do
    nodeid=`expr $((0x620)) + $i`
    nickname=hul02sft-`expr + $i`
    dataport=`expr 9010 + $i` 
    sitcp_ip=192.168.11.`expr 200 + $i`
    min_window=0
    max_window=125
    only_leading=0
    master=--slave
#    if [ $i -eq 1 ]; then
#	master=--master
#    fi

    $bin_dir/frontend.sh \
	$nickname \
	$nodeid \
	$dataport \
	$sitcp_ip \
	$min_window \
	$max_window \
	$only_leading \
	$master \
	>/dev/null 2>/dev/null &
done
