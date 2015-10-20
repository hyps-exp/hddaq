#!/bin/sh

bin_dir=$(cd $(dirname $0); pwd -P)/../bin
script_dir=$(cd $(dirname $0); pwd -P)/../script
frontend=fe_easiroc02_frontend

if [ $# != 3 ]; then
    echo "Usage : $(basename $0) --nickname=NICK_NAME --nodeid=NODE_ID --data-port=DATA_PORT"
    exit
fi

nickname=$1
nodeid=$2
dataport=$3

while true
do
    if [ -d $script_dir/$nickname ]; then
	cd $script_dir/$nickname
	$bin_dir/$frontend \
	    --nickname=$nickname  \
	    --nodeid=$nodeid      \
	    --data-port=$dataport \
	    # --run-number-nolock
    else
	echo "- cannot find directory : $nickname"
	exit
    fi
    sleep 1
done
