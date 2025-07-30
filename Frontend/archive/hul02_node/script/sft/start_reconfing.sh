#!/bin/bash

shell_name=reconfig_fpga.sh
session_name=reconfig_sft

for i in $(seq 1 9)
do
    sitcp_ip=192.168.11.`expr 200 + $i`
    tmux new-session -d -s ${session_name}_$i "sh $shell_name $sitcp_ip"
done

while sleep 1;
do
    if [ ! "`tmux ls | grep $session_name`" ]; then
	echo "#D: $session_name is finished"
	break;
    else
	echo "#D: waiting reconfig..."
    fi
done
