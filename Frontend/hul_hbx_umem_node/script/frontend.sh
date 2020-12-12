#!/bin/sh

script_dir=$(dirname `readlink -f $0`)
bin_dir=$script_dir/../bin
frontend=hul_hbx_frontend_umem

if [ $# != 5 ]; then
    echo "Usage : $(basename $0) --nickname=NICK_NAME --nodeid=NODE_ID --data-port=DATA_PORT --sctcp-ip=SiTCP_IP --master=master/slave"
    exit
fi

nickname=$1
nodeid=$2
dataport=$3
sitcp_ip=$4
master=$5

while true
do
$bin_dir/$frontend \
    --nickname=$nickname  \
    --nodeid=$nodeid      \
    --data-port=$dataport \
    --sitcp-ip=$sitcp_ip \
    $master \ 
    # --run-number-nolock

    sleep 1
done
