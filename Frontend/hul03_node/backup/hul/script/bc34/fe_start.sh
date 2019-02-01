#!/bin/sh

bin_dir=$(dirname `readlink -f $0`)

cd $bin_dir

#$bin_dir/message.sh & #> /dev/null 2> /dev/null &
./message.sh > /dev/null 2> /dev/null &

sleep 1

for i in $(seq 1 10)
do
    nodeid=`expr $((0x610)) + $i`
    nickname=hulbc-`expr + $i`
    dataport=`expr 9000 + $i` 
    sitcp_ip=192.168.11.`expr 30 + $i`
    min_window=0
    max_window=188
    only_leading=0
    master=--slave
    if [ $i -eq 10 ]; then
	master=--master
    fi

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
