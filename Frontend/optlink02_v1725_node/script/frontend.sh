#!/bin/sh

nickname=optlink02
#nodeid=$((0x1B9F))
nodeid=5002
dataport=9000
frontend=optlink02_v1725_frontend

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
