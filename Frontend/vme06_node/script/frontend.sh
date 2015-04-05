#!/bin/sh

nickname=vme06
nodeid=$((0x206))
frontend=vme06_frontend

bin_dir=$(cd $(dirname $0); pwd -P)/../bin
#echo ${bin_dir}

if [ $$ -ne $(pgrep -fo $0) ]; then
    echo "$0 is already running."
    exit 1
fi

while true
do
    echo -e "\n\n"${frontend}
    ${bin_dir}/${frontend} --nickname=${nickname} --nodeid=${nodeid}
    sleep 1
done
