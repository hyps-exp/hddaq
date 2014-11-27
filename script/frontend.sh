#!/bin/sh

nickname=vme01
nodeid=$((0x201))

bin_dir=$(cd $(dirname $0); pwd)/../Frontend/bin
echo ${bin_dir}

while true
do
  ${bin_dir}/frontend --nickname=${nickname} --nodeid=${nodeid}
  sleep 1
done
