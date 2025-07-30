#!/bin/sh

bin_dir=$(dirname `readlink -f $0`)

cd $bin_dir

#$bin_dir/message.sh & #> /dev/null 2> /dev/null &
./message.sh > /dev/null 2> /dev/null &

sleep 1
#start hulRM

#$HOME/HUL_Scaler/bin/set_nimio 192.168.10.62

# enable mezzanine slot
en_slot=(0x3)
sitcp_ip=("10.101")
#sitcp_ip=("1.69")

#start hulhrtdc
for i in $(seq 0 0)
do
    nodeid=`expr $((0x601)) + $i`
    nickname=hul04hr-`expr 1 + $i`
    dataport=`expr 9001 + $i`
    min_window=10
    max_window=2000

    $bin_dir/frontend_hrtdc.sh \
	$nickname \
	$nodeid \
	$dataport \
	192.168.${sitcp_ip[$i]} \
	$min_window \
	$max_window \
	${en_slot[$i]} \
	>/dev/null 2>/dev/null &
done
