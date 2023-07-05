#!/bin/sh

easiroc_start=0
easiroc_end=9

#ip address: see daq_info

bin_dir=$(cd $(dirname $0); pwd -P)

cd $bin_dir

$bin_dir/message.sh > /dev/null 2> /dev/null &

sleep 1

for i in $(seq $easiroc_start $easiroc_end)
do
  nodeid=`expr $((0xea00)) + $i`
  nickname=easiroc`printf "%02d" $i`
  dataport=`expr 9000 + $i`

  $bin_dir/frontend.sh \
      $nodeid \
      $nickname \
      $dataport > /dev/null 2>/dev/null &
done

exit 0