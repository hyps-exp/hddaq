#!/bin/sh

bin_dir=$(dirname `readlink -f $0`)

cd $bin_dir

#$bin_dir/message.sh & #> /dev/null 2> /dev/null &
./message.sh > /dev/null 2> /dev/null &

#ctrl_bin=$HOME/hul_software/HRM/bin/set_nimio
master_ip=192.168.11.101
#$ctrl_bin $master_ip

sleep 1

#for i in $(seq 1 7)
for i in $(seq 1 12)
do

#    if [ \( $i -eq 3 \) -o \( $i -eq 4 \) -o  \( $i -eq 6 \) ]; then
 #       continue
  #  fi

    nodeid=`expr $((0x610)) + $i`
    nickname=hul02sdc-`expr + $i`
    dataport=`expr 9000 + $i` 
    sitcp_ip=192.168.11.`expr 100 + $i`
    min_window=25
    #max_window=150
    max_window=200
    only_leading=0
    master=--slave
    if [ $i -eq 1 ]; then
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
