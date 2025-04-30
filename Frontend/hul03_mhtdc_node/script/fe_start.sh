#!/bin/sh

bin_dir=$(dirname `readlink -f $0`)

cd $bin_dir

$bin_dir/message.sh  > /dev/null 2> /dev/null &
# ./message.sh > /dev/null 2> /dev/null &

# ctrl_bin=$HOME/hul_software/HRM/bin/set_nimio
# master_ip=192.168.1.40
# $ctrl_bin $master_ip
# $ctrl_bin 192.168.1.41

sleep 1

for i in $(seq 51 51) # MH-TDC in VME Crate-5
do
    nodeid=`expr 2000 + $i`
    nickname=hul03_misc-$i
    dataport=`expr 9000 + $i`
    sitcp_ip=192.168.11.$i
    min_window=6
    max_window=150
    # max_window=600
    only_leading=0
    master=--slave
    if [ $i -eq 31 ]; then
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
