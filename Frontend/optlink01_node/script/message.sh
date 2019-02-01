#!/bin/sh

bin_dir=$(dirname `readlink -f $0`)/../../../Message/bin

if [ $$ -ne $(pgrep -fo $0) ]; then
  echo "$0 is already running."
  exit 1
fi

while true
do
  ${bin_dir}/msgd
  sleep 1
done
