#!/bin/sh

bin_dir=$(cd $(dirname $0); pwd -P)

$bin_dir/message.sh > /dev/null 2> /dev/null &

sleep 1

#for i in $(seq 1 2)
for i in $(seq 1 3)
do
    nodeid=`expr $((0xea10)) + $i`
    nickname=easiroc`expr 10 + $i`
    dataport=`expr 9000 + $i`
    $bin_dir/frontend.sh $nickname $nodeid $dataport >/dev/null 2>/dev/null &
#    $bin_dir/frontend.sh $nickname $nodeid $dataport &
done
