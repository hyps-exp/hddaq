#!/bin/sh

bin_dir=$(cd $(dirname $0); pwd -P)/../../../Message/bin

if [ $$ -ne $(pgrep -fo $0) ]; then
  echo "$(basename $0) is already running."
  exit 1
fi

while true
do
  ${bin_dir}/msgd
  sleep 1
done

