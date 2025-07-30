#!/bin/sh

echo $0

bin_dir=$(cd $(dirname $0); pwd -P)/../../../../Message/bin
msgd=$bin_dir/msgd

echo $0

if [ $$ -ne $(pgrep -fo $0) ]; then
    echo "$0 is already running."
    exit
fi

while true
do
    $msgd
    sleep 1
done
