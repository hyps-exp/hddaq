#!/bin/sh

bin_dir=$(dirname `readlink -f $0`)

cd $bin_dir

#$bin_dir/message.sh & #> /dev/null 2> /dev/null &
./message.sh > /dev/null 2> /dev/null &

ctrl_bin=$HOME/work/HUL_MHTDC/bin/set_nimio
master_ip=192.168.1.40
$ctrl_bin $master_ip


sleep 1

for i in $(seq 1 10)
do
    nodeid=`expr 130 + $i`
    nickname=hulbc-`expr + $i`
    dataport=`expr 9000 + $i` 
    sitcp_ip=192.168.1.`expr 30 + $i`
    min_window=6
    max_window=75
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
