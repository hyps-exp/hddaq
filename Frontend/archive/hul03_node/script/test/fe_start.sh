#!/bin/sh

bin_dir=$(dirname `readlink -f $0`)

cd $bin_dir

#$bin_dir/message.sh & #> /dev/null 2> /dev/null &
./message.sh > /dev/null 2> /dev/null &

#ctrl_bin=$HOME/hul_software/HRM/bin/set_nimio
#master_ip=192.168.1.40
#$ctrl_bin $master_ip
#$ctrl_bin 192.168.1.41

sleep 1

for i in $(seq 0 0)
do
   # if [ $i -eq 3 ]; then
#	continue
 #   fi

## original ##
#    nodeid=`expr 1700 + $i`
#    if [ $i -le 6 ]; then
#	nickname=hul03bc-`expr + $i`
#    else
#	nickname=hul03sdc-`expr + $i - 6`
#    fi
## original by here ##

    nodeid=`expr 1538 + $i`
    nickname=hul03-`expr 2 + $i`
    
    dataport=`expr 9002 + $i` 
    #    sitcp_ip=192.168.1.`expr 30 + $i`
    sitcp_ip=192.168.10.`expr 102 + $i`    
    min_window=0
    max_window=150
    only_leading=0
    master=--slave
    if [ $i -eq 10 ]; then
	master=--master
    fi
    if [ $i -eq 11 ]; then
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
