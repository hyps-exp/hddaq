#!/bin/sh

bin_dir=$(dirname `readlink -f $0`)

cd $bin_dir

$bin_dir/message.sh > /dev/null 2> /dev/null &

sleep 1

reg_dir=${HOME}/vme-easiroc-registers
#reg_dir=default
adc=on
tdc=on

for i in $(seq 19 22) $(seq 35 38) $(seq 54 58) $(seq 75 80)
# for i in $(seq 20 22) $(seq 35 38) $(seq 55 58) $(seq 75 80) # for Hold timing check, exclude 19, 54

do
    nodeid=`expr 4000 + $i`
    nickname=veasiroc02-$i
    dataport=`expr 9000 + $i`
    sitcp_ip=192.168.11.$i
    module_num=$i

    $bin_dir/frontend.sh \
	$nickname \
	$nodeid \
	$dataport \
	$sitcp_ip \
	$module_num \
	$reg_dir \
	$adc \
	$tdc \
	>/dev/null 2>/dev/null &
done
