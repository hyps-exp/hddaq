#!/bin/sh

clite_start=1
clite_end=9

bin_dir=$(cd $(dirname $0); pwd -P)

cd $bin_dir

$bin_dir/message.sh > /dev/null 2>/dev/null &

sleep 1

for i in $(seq $clite_start $clite_end)
do
    nip=`echo "$i + 30" | bc`
    nport=`echo "$i -1 + 9000" | bc`
    $bin_dir/frontend.sh \
	192.168.1.$nip \
	192.168.2.$nip \
	clite$i \
	1$nip \
	$nport > /dev/null 2>/dev/null &
done

exit 0