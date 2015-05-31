#!/bin/sh

(cd /home/DAQ/hddaq/pro/Frontend/easiroc_node/script ; ./msgd.sh) > /dev/null 2> /dev/null &
sleep 1

for i in $(seq 0 9)
  do
  (cd /home/DAQ/hddaq/pro/Frontend/easiroc_node/script/easiroc0$i ; ./frontend.sh 1110$i easiroc$i 900$i)  > /dev/null &#2>/dev/null &
done

exit 0