#!/bin/sh

bin_dir=$(dirname `readlink -f $0`)

cd $bin_dir

#$bin_dir/message.sh & #> /dev/null 2> /dev/null &
./message.sh > /dev/null 2> /dev/null &

#ctrl_bin=$HOME/work/HUL_MHTDC/bin/set_nimio
#master_ip=192.168.1.40
#$ctrl_bin $master_ip


sleep 1


nodeid=2004
nickname=hul_hbx_mhtdc-1
dataport=9004 
sitcp_ip=192.168.12.14
min_window=0
max_window=1040
only_leading=1
master=--slave

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
