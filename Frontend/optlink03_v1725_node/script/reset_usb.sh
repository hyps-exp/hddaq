#!/bin/sh

while read -r line; do
  echo $line
  bus=$(echo $line | awk '{print $2}')
  dev=$(echo $line | awk '{print $4}' | tr -d ':')
  sys_path=$(readlink -f /dev/bus/usb/$bus/$dev)
  dev_name=$(basename $sys_path)
  echo $sys_path  $dev_name
done < <(lsusb | grep CAEN)
