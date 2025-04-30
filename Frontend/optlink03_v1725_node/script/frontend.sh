#!/bin/sh

nickname=optlink03
#nodeid=$((0x1B9F))
nodeid=5003
dataport=9000
frontend=optlink03_v1725_frontend

bin_dir=$(dirname `readlink -f $0`)/../bin

if [ $$ -ne $(pgrep -fo $0) ]; then
    echo "$0 is already running."
    exit 1
fi

while true
do
    echo -e "\n\n"${frontend}
    ${bin_dir}/${frontend} --nickname=${nickname} --nodeid=${nodeid} --data-port=$dataport
    sleep 1
done
