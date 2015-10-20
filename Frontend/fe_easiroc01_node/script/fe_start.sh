#!/bin/sh

(cd /home/DAQ/hddaq/pro/Frontend/easiroc_node/script ; ./msgd.sh) > /dev/null 2> /dev/null &
sleep 1

for i in $(seq 0 9)
  do
  nodeid=`expr $((0xea00)) + $i`
  (cd /home/DAQ/hddaq/pro/Frontend/easiroc_node/script/easiroc0$i ; ./frontend.sh $nodeid easiroc$i 900$i)  > /dev/null &#2>/dev/null &
done

exit 0