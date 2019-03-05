#!/bin/sh

bin_dir=$(dirname `readlink -f $0`)

cd $bin_dir

#$bin_dir/message.sh & #> /dev/null 2> /dev/null &
./message.sh > /dev/null 2> /dev/null &

sleep 1
#start hulRM
$HOME/HUL_RM/bin/set_nimio 192.168.10.60

#start hulhrtdc
for i in $(seq 1 2)
do
    nodeid=`expr $((0x605)) + $i`
    nickname=hul01hr-`expr + $i`
    dataport=`expr 9009 + $i` 
    sitcp_ip=192.168.10.`expr 65 + $i`
    min_window=35
    max_window=60

    $bin_dir/frontend_hrtdc.sh \
	$nickname \
	$nodeid \
	$dataport \
	$sitcp_ip \
	$min_window \
	$max_window \
	>/dev/null 2>/dev/null &
done
