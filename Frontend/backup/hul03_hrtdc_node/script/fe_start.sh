#!/bin/sh

bin_dir=$(dirname `readlink -f $0`)

cd $bin_dir

#$bin_dir/message.sh & #> /dev/null 2> /dev/null &
./message.sh > /dev/null 2> /dev/null &

sleep 1
#start hulRM

# enable mezzanine slot (0th value is dummy)
# bit0: up, bit1: down
en_slot=(0x0, 0x3, 0x1)

#start hulhrtdc
for i in $(seq 1 2)
do
    nodeid=`expr 1720 + $i`
    nickname=hul03hr-`expr + $i`
    dataport=`expr 9020 + $i`
    sitcp_ip=192.168.1.`expr 68 + $i`
    min_window=0
    max_window=200

    $bin_dir/frontend_hrtdc.sh \
	$nickname \
	$nodeid \
	$dataport \
	$sitcp_ip \
	$min_window \
	$max_window \
	${en_slot[$i]} \
	>/dev/null 2>/dev/null &
done
