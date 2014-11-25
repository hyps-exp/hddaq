#!/bin/sh

nickname=vme03
nodeid=$((0x203))

bin_dir=$(cd $(dirname $0); pwd)/../Frontend/bin
echo ${bin_dir}

while true
do
  ${bin_dir}/frontend --nickname=${nickname} --nodeid=${nodeid}
  sleep 1
done
