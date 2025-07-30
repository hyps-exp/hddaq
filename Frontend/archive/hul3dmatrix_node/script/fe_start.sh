#!/bin/sh

bin_dir=$(cd $(dirname $0); pwd -P)

cd $bin_dir

$bin_dir/message.sh > /dev/null 2> /dev/null &

sleep 1

nodeid=33750
nickname=hul3dmtx
dataport=9000
$bin_dir/frontend.sh $nickname $nodeid $dataport >/dev/null 2>/dev/null &
#$bin_dir/frontend.sh $nickname $nodeid $dataport
