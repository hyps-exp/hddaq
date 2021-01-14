#!/bin/sh

bin_dir=$(dirname `readlink -f $0`)

# ${bin_dir}/message.sh >/dev/null 2>/dev/null &

sleep 1;

#for i in $(seq 1 1)
for i in $(seq 0 7)
do
    nickname=cobo$i
    nodeid=`expr $((0xcb0)) + $i`
    dataport=`expr 9000 + $i`
    coboid=$i

    ${bin_dir}/frontend.sh \
	$nickname \
	$nodeid \
	$dataport \
	$coboid \
	>/dev/null 2>/dev/null &
done
