#!/bin/sh

bin_dir=$(dirname `readlink -f $0`)

cd $bin_dir

#$bin_dir/message.sh & #> /dev/null 2> /dev/null &
./message.sh > /dev/null 2> /dev/null &

sleep 1
#start hulRM

#$HOME/HUL_Scaler/bin/set_nimio 192.168.10.62

# enable mezzanine slot (0th value is dummy)
en_slot=(0x0, 0x3, 0x3, 0x1, 0x3)

#start hulhrtdc
for i in $(seq 1 1)
#for i in $(seq 1 3)
#for i in $(seq 2 2)
#for i in $(seq 2 3)
do
    nodeid=`expr 9030 + $i`
    nickname=hul01hr-`expr + $i`
    dataport=`expr 9030 + $i`
    sitcp_ip=192.168.11.`expr 130 + $i`
    min_window=10
    max_window=100

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
