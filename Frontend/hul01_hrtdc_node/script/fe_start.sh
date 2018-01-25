#!/bin/sh

bin_dir=$(dirname `readlink -f $0`)

cd $bin_dir

#$bin_dir/message.sh & #> /dev/null 2> /dev/null &
./message.sh > /dev/null 2> /dev/null &

sleep 1

for i in $(seq 1 1)
do
    nodeid=`expr $((0x602)) + $i`
    nickname=hul01hr-`expr + $i`
    dataport=`expr 9001 + $i` 
    sitcp_ip=192.168.10.`expr 63 + $i`
    min_window=10
    max_window=50

    $bin_dir/frontend_hrtdc.sh \
	$nickname \
	$nodeid \
	$dataport \
	$sitcp_ip \
	$min_window \
	$max_window 
#	>/dev/null 2>/dev/null &
done
