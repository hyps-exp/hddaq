#!/bin/sh

bin_dir=$(cd $(dirname $0); pwd -P)/../bin
script_dir=$(cd $(dirname $0); pwd -P)/../script
frontend=hul3dmatrix_frontend

if [ $# != 3 ]; then
    echo "Usage : $(basename $0) --nickname=NICK_NAME --nodeid=NODE_ID --data-port=DATA_PORT"
    exit
fi

nickname=$1
nodeid=$2
dataport=$3

while true
do
$bin_dir/$frontend \
    --nickname=$nickname  \
    --nodeid=$nodeid      \
    --data-port=$dataport \
    # --run-number-nolock

    sleep 1
done
