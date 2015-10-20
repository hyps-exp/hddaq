#!/bin/sh

for i in $(seq 0 9)
do
  rm ./easiroc0$i/coredump.dat
done
