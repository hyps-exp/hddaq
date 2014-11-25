#!/bin/sh

bin_dir=$(cd $(dirname $0); pwd)/../Message/bin
echo ${bin_dir}

while true
do
  ${bin_dir}/msgd
  sleep 1
done
