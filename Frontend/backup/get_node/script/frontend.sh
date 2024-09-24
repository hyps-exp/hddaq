#!/bin/sh

frontend=get_frontend
bin_dir=$(dirname `readlink -f $0`)/../bin

# if [ $$ -ne $(pgrep -fo $0) ]; then
#     echo "$0 is already running."
#     exit 1
# fi

nickname=$1
nodeid=$2
dataport=$3
coboid=$4

while true
do
    echo -e "\n\n"${frontend}
    ${bin_dir}/${frontend} \
	--nickname=${nickname} \
	--nodeid=${nodeid} \
	--data-port=${dataport} \
	--coboid=${coboid}
    sleep 1
done
