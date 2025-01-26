#!/bin/sh

bin_dir=$(dirname `readlink -f $0`)

cd $bin_dir

$bin_dir/message.sh > /dev/null 2> /dev/null &

sleep 1

reg_dir=${HOME}/vme-easiroc-registers
#reg_dir=default
adc=on
adc_off=off
tdc=on

for i in $(seq 90 94)

do
    nodeid=`expr 4000 + $i`
    nickname=veasiroc05-$i
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

    # if [ $i -ge 96 -a $i -le 102 ]; then
    # 	$bin_dir/frontend.sh \
	# 	    $nickname \
	# 	    $nodeid \
	# 	    $dataport \
	# 	    $sitcp_ip \
	# 	    $module_num \
	# 	    $reg_dir \
	# 	    $adc_off \
	# 	    $tdc \
	# 	    >/dev/null 2>/dev/null &
    # else
    # 	$bin_dir/frontend.sh \
	# 	    $nickname \
	# 	    $nodeid \
	# 	    $dataport \
	# 	    $sitcp_ip \
	# 	    $module_num \
	# 	    $reg_dir \
	# 	    $adc \
	# 	    $tdc \
	# 	    >/dev/null 2>/dev/null &
    # fi

done
