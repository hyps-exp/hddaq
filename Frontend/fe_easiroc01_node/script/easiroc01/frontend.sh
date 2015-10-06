#!/bin/sh

while true
do
  /home/DAQ/hddaq/feeasiroc --nodeid=$1 --nickname=$2 --data-port=$3 # --run-number-nolock
  sleep 1
done