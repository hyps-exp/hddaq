#!/bin/bash

bin_dir=$HOME/work/HUL_MHTDC/bin
bin_name=reconfig_fpga
sitcp_ip=$1

$bin_dir/$bin_name $sitcp_ip

while sleep 1;
do
    if [ "`ping -c 1 -q $sitcp_ip | cut -d' ' -f1 | grep rtt`" ]; then
	break
    fi
done

echo "#D: Module($sitcp_ip) reconfig done."
