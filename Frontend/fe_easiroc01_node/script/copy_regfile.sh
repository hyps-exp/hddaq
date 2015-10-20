#!/bin/sh

for i in $(seq 0 9)
  do
  cp EnHGSSh.txt ./easiroc0$i/setup/
  cp ReadSC_Channel.txt ./easiroc0$i/setup/
done