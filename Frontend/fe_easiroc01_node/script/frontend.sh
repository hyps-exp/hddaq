#!/bin/sh

bin_dir=$(cd $(dirname $0); pwd -P)/../bin
script_dir=$(cd $(dirname $0); pwd -P)/../script
frontend=fe_easiroc01_frontend

if [ $# != 3 ]; then
    echo "Usage: $(basename $0) [NODEID] [NICNAME] [DATAPORT]"
    exit
fi

nodeid=$1
nickname=$2
dataport=$3

while true
do
  if [ -d $script_dir/$nickname ]; then
      cd $script_dir/$nickname
      $bin_dir/$frontend \
	  --nodeid=$nodeid \
	  --nickname=$nickname \
	  --data-port=$dataport \
	  # --run-number-nolock
  else
      echo "- cannot find directory : $nickname"
      exit
  fi
  sleep 1
done
