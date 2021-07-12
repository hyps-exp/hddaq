#!/bin/sh

nickname=vme08
nodeid=$((0x208))
frontend=vme08_frontend

bin_dir=$(cd $(dirname $0); pwd)/../bin
#echo ${bin_dir}

if [ $$ -ne $(pgrep -fo $0) ]; then
    echo "$0 is already running."
    exit 1
fi

while true
do
    echo -e "\n\n"${frontend}
    ${bin_dir}/${frontend} --nickname=${nickname} --nodeid=${nodeid}
    # --ignore-nodeprop-update
    sleep 1
done
