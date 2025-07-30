#!/bin/sh

nickname=ccnet01
nodeid=$((0xcc01))
frontend=ccnet01_frontend

bin_dir=$(cd $(dirname $0); pwd)/../bin
#echo ${bin_dir}

if [ $$ -ne $(pgrep -fo $0) ]; then
    echo "$0 is already running."
    exit 1
fi

while true
do
    echo -e "\n\n"${frontend}
    ${bin_dir}/${frontend} --nickname=${nickname} --nodeid=${nodeid} #--run-number-nolock
    sleep 1
done
