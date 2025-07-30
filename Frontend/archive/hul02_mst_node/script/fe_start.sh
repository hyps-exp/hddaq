#!/bin/sh

bin_dir=$(dirname `readlink -f $0`)

cd $bin_dir

#$bin_dir/message.sh & #> /dev/null 2> /dev/null &
./message.sh > /dev/null 2> /dev/null &

sleep 1

for i in $(seq 1 1)
do
    nodeid=`expr $((0x640)) + $i`
    nickname=hul02mst-`expr + $i`
    dataport=`expr 9030 + $i` 
    sitcp_ip=192.168.11.`expr 11 + $i`
    min_window=20
    max_window=35
    prescale=100
#    timer=300
    timer=450
    bypass=0
    master=--slave
    if [ $i -eq 1 ]; then
	master=--master
    fi

    $bin_dir/frontend_mst.sh \
	$nickname \
	$nodeid \
	$dataport \
	$sitcp_ip \
	$min_window \
	$max_window \
	$prescale \
        $timer \
	$bypass \
	$master \
	>/dev/null 2>/dev/null &
done
