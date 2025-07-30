#!/bin/sh

bin_dir=$(cd $(dirname $0); pwd -P)

cd $bin_dir

for i in $(seq 0 9)
  do
  nickname=easiroc`printf "%02d" $i`
  cp -v EnHGSSh.txt $nickname/setup/
  cp -v ReadSC_Channel.txt $nickname/setup/
done
