#!/bin/sh

script_dir=$(dirname `readlink -f $0`)
bin_dir=$script_dir/../bin
frontend=hul_mst_frontend

if [ $# != 4 ]; then
    echo "Usage : $(basename $0) --nickname=NICK_NAME --nodeid=NODE_ID --data-port=DATA_PORT --sctcp-ip=SiTCP_IP"
    exit
fi

nickname=$1
nodeid=$2
dataport=$3
sitcp_ip=$4

while true
do
$bin_dir/$frontend \
    --nickname=$nickname  \
    --nodeid=$nodeid      \
    --data-port=$dataport \
    --sitcp-ip=$sitcp_ip  
    #--ignore-nodeprop-update 

    sleep 1
done
