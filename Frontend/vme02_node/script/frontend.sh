#!/bin/sh

nickname=vme02
nodeid=$((0x202))
frontend=vme02_frontend

bin_dir=$(dirname `readlink -f $0`)/../bin

if [ $$ -ne $(pgrep -fo $0) ]; then
    echo "$0 is already running."
    exit 1
fi

while true
do
    echo -e "\n\n"${frontend}
    ${bin_dir}/${frontend} \
	--nickname=${nickname} \
	--nodeid=${nodeid}
    sleep 1
done
