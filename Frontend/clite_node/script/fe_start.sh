#!/bin/sh

./msgd.sh > /dev/null 2>/dev/null &
sleep 1

for i in $(seq 1 14)
do
    nip=`echo "$i + 30" | bc`
    nport=`echo "$i -1 + 9000" | bc`
    ./frontend.sh 192.168.1.$nip 192.168.2.$nip clite$i 1$nip $nport > /dev/null 2>/dev/null &
#    ./frontend.sh 192.168.1.$nip 192.168.2.$nip clite$i 1$nip $nport &
done

exit 0