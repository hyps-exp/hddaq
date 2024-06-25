#!/bin/sh

bin_dir=$(dirname `readlink -f $0`)

cd $bin_dir

$bin_dir/message.sh > /dev/null 2> /dev/null &

sleep 1

# enable block register (0th value is dummy)
en_block=(0xb 0x3 0x3 0x3 0x3 0x0)
# ip=(61 62 63 60)

#a=(1 2)
#a=(1 2 5)
#a=(1 2 5 0)
#for i in ${a[@]}
#for i in $(seq 0 2)
for i in $(seq 1 1)
do
    nodeid=`expr 9050 + $i`
    nickname=hul01scr-`expr 1 + $i`
    dataport=`expr 9050 + $i`
    sitcp_ip=192.168.11.151
    # master=--master
    master=--slave

    $bin_dir/frontend.sh \
	$nickname \
	$nodeid \
	$dataport \
	$sitcp_ip \
	$master \
	${en_block[$i]} \
	>/dev/null 2>/dev/null &
done
