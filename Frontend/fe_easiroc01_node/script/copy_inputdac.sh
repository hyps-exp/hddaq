#!/bin/sh

bin_dir=$(cd $(dirname $0); pwd -P)

param_dir=/home/DAQ/bftparam/201510_e0705/inputdac_g30_temp23.0

for i in $(seq 0 9)
  do
  nickname=easiroc`printf "%02d" $i`
  param_file=$param_dir/Input8bitDAC_$i.txt

  cp -v $param_file $bin_dir/$nickname/setup/Input8bitDAC.txt
done
