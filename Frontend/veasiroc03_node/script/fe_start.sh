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
do
  if [ $i -eq 18 ]; then continue; fi
  # if [ $i -eq 67 ]; then continue; fi
      
    nodeid=`expr 2000 + $i`
    nickname=veasiroc$i
    dataport=`expr 9100 + $i` 
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
